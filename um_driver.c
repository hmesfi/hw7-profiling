/********************************************************************
 *
 *                     um_driver.c
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *     
 *     Implementation for um_driver module. This is our driver
 *     program that allows us to fully emulate the Universal Machine, 
 *     by reading in instructions from a file and utilizing our 
 *     UM module.
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <sys/stat.h>
#include <seq.h>

#define HINT 100
#define INSTRUCTION_LEN 4

#define NUM_REGISTERS 8
#define OP_CODE_LEN 4
#define REGISTER_LEN 3
#define VALUE_LEN 25
#define HINT 100


typedef struct seg_T {
        uint32_t *segments; 
        int seg_length;
} *seg_T;

typedef struct mem_T {
        seg_T *mapped_ids;
        uint32_t mapped_used;
        uint32_t mapped_length;

        Seq_T unmapped_ids;
} *mem_T;

typedef struct um_T {
        mem_T memory;
        uint32_t registers[NUM_REGISTERS];
        uint32_t program_counter;
        unsigned ra, rb, rc;
} um_T;

int main(int argc, char *argv[])
{
        if (argc != 2) {
                return EXIT_FAILURE;
        }

        FILE *fp = fopen(argv[1], "rb");
        assert(fp != NULL);

        um_T um;

        um.memory = malloc(sizeof(*(um.memory)));
        um.memory->mapped_used = 0;
        um.memory->mapped_length = 128;
        um.memory->mapped_ids = calloc(um.memory->mapped_length, 
                                        sizeof(seg_T));
        um.memory->unmapped_ids = Seq_new(HINT);

        /* 
         * confirm scan of provided program successful, then get info 
         * Note: size of file divided by 4 since 32-bit word == 4 bytes,
         * so this gives us total number of 32-bit words
         */
        struct stat program_info;
        stat(argv[1], &program_info);

        /* Note: will only read up to the last complete word in file */
        const int total_words = program_info.st_size / 4;

        seg_T program = malloc(sizeof(*program));
        assert(program != NULL);
        
        uint32_t *m0 = calloc(total_words, sizeof(m0));
        assert(m0 != NULL);

        program->segments = m0;
        program->seg_length = total_words;

        um.memory->mapped_ids[0] = program;
        (um.memory->mapped_used)++;

        /* write each word in segment one byte at a time (big endian order) */
        uint32_t *segment_zero = (um.memory->mapped_ids[0])->segments;
        int c = fgetc(fp);
        uint32_t um_instruction, mask;
        
        for (int word = 0; word < total_words; word++) {
                
                um_instruction = 0;

                for (int i = INSTRUCTION_LEN - 1; i >= 0; i--) {
                        mask = c;
                        mask = mask << (i * (INSTRUCTION_LEN * 2));
                        um_instruction = um_instruction | mask;
                        c = fgetc(fp);
                }

                segment_zero[word] = um_instruction;
        }

        for (int i = 0; i < NUM_REGISTERS; i++) {
                um.registers[i] = 0;
        }

        um.program_counter = 0;

        uint32_t program_length = total_words;

        /* UM will keep executing commands until it reaches the end of m[0]. */
        while (um.program_counter < program_length) {

                um_instruction = (um.memory->mapped_ids[0])->segments[um.program_counter];
                uint32_t op_code = um_instruction >> 28;

                /* Ensures that the instruction is valid. */
                assert(op_code < 14);

                if (op_code != 13) {
                        um.ra = um_instruction << 23 >>29;
                        um.rb = um_instruction << 26 >> 29;
                        um.rc = um_instruction & 0x7;
                }

                if (op_code == 7) {
                        /* Frees the array of words inside the memory segment. */
                        for (uint32_t i = 0; i < um.memory->mapped_length; i++) {
                                seg_T delete_segment = um.memory->mapped_ids[i];
                                if (delete_segment != NULL) {
                                        free(delete_segment->segments);
                                        free(delete_segment);
                                }
                        }

                        //Seq_free(&(mem_segments->mapped_ids));
                        Seq_free(&(um.memory->unmapped_ids));
                        free(um.memory);
                        break;              
                }

                //execute_command(&um, um_instruction, op_code);

                // switch (op_code)
                // {
                if (op_code == 0) { 
                        if (um.registers[um.rc] != 0) {
                                um.registers[um.ra] = um.registers[um.rb];
                        }
                } else if (op_code == 1) { 
                        um.registers[um.ra] = (um.memory->mapped_ids[um.registers[um.rb]])->
                                                                segments[um.registers[um.rc]];
                } else if (op_code == 2){
                        // segmented_store(um.memory, um.registers[um.ra], 
                        // um.registers[um.rb], um.registers[um.rc]);
                        (um.memory->mapped_ids[um.registers[um.ra]])->segments[um.registers[um.rb]] = um.registers[um.rc];
                } else if (op_code == 3) { 
                        um.registers[um.ra] = um.registers[um.rb] + um.registers[um.rc]; 
                } else if (op_code == 4) {
                        um.registers[um.ra] = um.registers[um.rb] * um.registers[um.rc];
                } else if (op_code == 5) {
                        um.registers[um.ra] = um.registers[um.rb] / um.registers[um.rc];
                } else if (op_code == 6) {
                        um.registers[um.ra] = ~(um.registers[um.rb] & um.registers[um.rc]);
                } else if (op_code == 8) {
                        seg_T new_segment = malloc(sizeof(*new_segment));
                        assert(new_segment != NULL);

                        uint32_t *new_seg = calloc(um.registers[um.rc], sizeof(new_seg));
                        assert(new_seg != NULL);

                        new_segment->segments = new_seg;
                        new_segment->seg_length = um.registers[um.rc];

                        uint32_t new_id;
                        
                        /* Attempts to reuse an unmapped segment identifier. */
                        if (Seq_length(um.memory->unmapped_ids) != 0) {
                                new_id = (uint32_t)(uintptr_t)
                                        Seq_remlo(um.memory->unmapped_ids);
                        } else {
                                new_id = um.memory->mapped_used;

                                if ((new_id == um.memory->mapped_length) &&
                                (um.memory->mapped_length != UINT32_MAX)) {
                                        um.memory->mapped_length =
                                                um.memory->mapped_length * 2;
                                        um.memory->mapped_ids = realloc(
                                                        um.memory->mapped_ids,
                                                        um.memory->mapped_length * sizeof(seg_T));
                                }
                        }

                        um.memory->mapped_ids[new_id] = new_segment;
                        um.memory->mapped_used++;

                        um.registers[um.rb] = new_id;
                } else if (op_code == 9) {
                        uint32_t id = um.registers[um.rc];
                        seg_T delete_segment = um.memory->mapped_ids[id];

                        free(delete_segment->segments);
                        free(delete_segment);
                        
                        um.memory->mapped_ids[id] = NULL;
                        (um.memory->mapped_used)--;
                        
                        Seq_addlo(um.memory->unmapped_ids, (void *)(uintptr_t)id);
                } else if (op_code == 10) { 
                        putchar(um.registers[um.rc]);
                } else if (op_code == 11) {
                        um.registers[um.rc] = getchar();
                } else if (op_code == 12){ 
                        uint32_t id = um.registers[um.rb];
                         if (id != 0) {
                                seg_T segment = um.memory->mapped_ids[id];
                                
                                seg_T delete_segment = um.memory->mapped_ids[0];

                                free(delete_segment->segments);
                                free(delete_segment);
                                
                                um.memory->mapped_ids[0] = NULL;
                                (um.memory->mapped_used)--;
                                
                                Seq_addlo(um.memory->unmapped_ids, (void *)(uintptr_t)0);

                                seg_T new_segment = malloc(sizeof(*new_segment));
                                assert(new_segment != NULL);

                                uint32_t *new_seg = calloc(segment->seg_length, sizeof(new_seg));
                                assert(new_seg != NULL);

                                new_segment->segments = new_seg;
                                new_segment->seg_length = segment->seg_length;

                                uint32_t new_id;
                                
                                /* Attempts to reuse an unmapped segment identifier. */
                                if (Seq_length(um.memory->unmapped_ids) != 0) {
                                        new_id = (uint32_t)(uintptr_t)
                                                Seq_remlo(um.memory->unmapped_ids);
                                } else {
                                        new_id = um.memory->mapped_used;

                                        if ((new_id == um.memory->mapped_length) &&
                                        (um.memory->mapped_length != UINT32_MAX)) {
                                                um.memory->mapped_length =
                                                        um.memory->mapped_length * 2;
                                                um.memory->mapped_ids = realloc(
                                                                um.memory->mapped_ids,
                                                                um.memory->mapped_length * sizeof(seg_T));
                                        }
                                }

                                um.memory->mapped_ids[new_id] = new_segment;
                                um.memory->mapped_used++;

                                // end of map seg        
                                
                                seg_T new_m0 = um.memory->mapped_ids[new_id];
                                // seg_T new_m0 = Seq_get(mem_segments->mapped_ids, new_id);

                                /* Copy the instructions from segment into the new program. */
                                for (int i = 0; i < segment->seg_length; i++) {
                                        new_m0->segments[i] = segment->segments[i];
                                }
                        }
                        um.program_counter = um.registers[um.rc];
                } else if (op_code == 13){
                        um.ra = um_instruction << OP_CODE_LEN >> 29;
                        um.registers[um.ra] = um_instruction << 7 >> 7;
                }

                /* If a new program is loaded, make sure that the program 
                 * length is changed. */
                if (op_code == 12) {
                        segment_zero = (um.memory->mapped_ids[0])->segments;
                        program_length = um.memory->mapped_ids[0]->seg_length;
                        continue;
                }

                um.program_counter++;
        }

        fclose(fp);

        return EXIT_SUCCESS; 
}

#undef NUM_REGISTERS
#undef OP_CODE_LEN
#undef REGISTER_LEN
#undef VALUE_LEN 


#undef HINT
#undef INSTRUCTION_LEN