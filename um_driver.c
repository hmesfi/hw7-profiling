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

// static FILE *open_or_abort(char *fname, char *mode);

// /* Performs all the functions of the UM. */
// void run_um(Seq_T instructions); 

/* Memory Segment functions */

/* Creates a memory instance. */
// mem_T mem_new();
// /* Frees a memory instance. */
// void mem_free(mem_T mem_segments);
// /* Maps a memory segment. */
// uint32_t map_segment(mem_T mem_segments, uint32_t num_words);
// /* Unmaps a memory segment. */
// void unmap_segment(mem_T mem_segments, uint32_t identifier);
// /* Creates the program m[0] and stores it in memory. */
// void initialize_program(mem_T mem_segments, int program_length);
// /* Retrieves a word from memory. */
// uint32_t segmented_load(mem_T mem_segments, uint32_t id, uint32_t index);
// /* Stores a word from memory. */
// void segmented_store(mem_T mem_segments, uint32_t id, uint32_t index, 
//                      uint32_t word);
// /* Creates the program m[0] using a segment stored in memory. */
// void load_program(mem_T mem_segments, uint32_t id);
// /* Gets the length of a segment in memory. */
// uint32_t segment_length(mem_T mem_segments, uint32_t id);
// /* Gets a word from a segment in memory. */
// uint32_t segment_word(mem_T mem_segments, uint32_t id, int index);


int main(int argc, char *argv[])
{
        if (argc != 2) {
                return EXIT_FAILURE;
        }

        //FILE *fp = open_or_abort(argv[1], "rb");
        FILE *fp = fopen(argv[1], "rb");
        //assert(fp != NULL);

        um_T um;

        //um.memory = mem_new();
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

        //initialize_program(um.memory, total_words);
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

                um_instruction = segment_zero[um.program_counter];
        
                uint32_t op_code = um_instruction >> 28;
                //get_opcode(um_instruction);

                /* Ensures that the instruction is valid. */
                assert(op_code < 14);

                if (op_code != 13) {
                        um.ra = um_instruction << 23 >>29;
                        um.rb = um_instruction << 26 >> 29;
                        um.rc = um_instruction & 0x7;
                }

                if (op_code == 7) {
                        //mem_free(um.memory);
        
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
                uint32_t value, id;
                seg_T new_segment, delete_segment;

                switch (op_code)
                {
                        case 0: 
                                if (um.registers[um.rc] != 0) {
                                        um.registers[um.ra] = um.registers[um.rb];
                                }
                                break;
                        case 1: 
                                // um.registers[um.ra] = segmented_load(um.memory,
                                //                         um.registers[um.rb], 
                                //                         um.registers[um.rc]);
                                um.registers[um.ra] = (um.memory->mapped_ids[um.registers[um.rb]])->
                                                                    segments[um.registers[um.rc]];
        
                                // assert(segment != NULL);

                                // return segment->segments[index];
                                break;
                        case 2: 
                                // segmented_store(um.memory, um.registers[um.ra], 
                                // um.registers[um.rb], um.registers[um.rc]);
                                (um.memory->mapped_ids[um.registers[um.ra]])->segments[um.registers[um.rb]] = um.registers[um.rc];
                                // seg_T segment = Seq_get(mem_segments->mapped_ids, id);
                                // assert(segment != NULL);

                                // segment->segments[index] = word;
                                break;
                        case 3: 
                                um.registers[um.ra] = um.registers[um.rb] + um.registers[um.rc]; 
                                break;
                        case 4:
                                um.registers[um.ra] = um.registers[um.rb] * um.registers[um.rc];
                                break;
                        case 5: 
                                um.registers[um.ra] = um.registers[um.rb] / um.registers[um.rc];
                                break;
                        case 6: 
                                um.registers[um.ra] = ~(um.registers[um.rb] & um.registers[um.rc]);
                                break;
                        case 8: 
                                new_segment = malloc(sizeof(*new_segment));
                                assert(new_segment != NULL);

                                uint32_t *new_seg = calloc(um.registers[um.rc], sizeof(new_seg));
                                assert(new_seg != NULL);

                                new_segment->segments = new_seg;
                                new_segment->seg_length = um.registers[um.rc];

                                uint32_t new_id;
                                
                                if (Seq_length(um.memory->unmapped_ids) != 0) {
                                        new_id = (uint32_t)(uintptr_t)
                                                Seq_remlo(um.memory->unmapped_ids);
                                } else {
                                        new_id = um.memory->mapped_used;

                                        if ((new_id == um.memory->mapped_length)
                                        &&(um.memory->mapped_length != UINT32_MAX)) {
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
                                
                                break;
                        case 9: 
                                // unmap_segment(um.memory, um.registers[um.rc]);
                                delete_segment = um.memory->mapped_ids[um.registers[um.rc]];
                                free(delete_segment->segments);
                                free(delete_segment);
                                
                                um.memory->mapped_ids[um.registers[um.rc]] = NULL;
                                (um.memory->mapped_used)--;
                                
                                Seq_addlo(um.memory->unmapped_ids, (void *)(uintptr_t)um.registers[um.rc]);
                                break;
                        case 10: 
                                putchar(um.registers[um.rc]);
                                break;
                        case 11: 
                                um.registers[um.rc] = getchar();
                                break;
                        case 12: 
                                //load_program(um.memory, um.registers[um.rb]);
                                id = um.registers[um.rb];
                                if (id != 0) {
                                        seg_T segment = um.memory->mapped_ids[id];
                                
                                        //unmap_segment(um.memory, 0);
                                        seg_T delete_segment = um.memory->mapped_ids[id];
                                        //assert(delete_segment != NULL);

                                        free(delete_segment->segments);
                                        free(delete_segment);
                                        
                                        um.memory->mapped_ids[id] = NULL;
                                        (um.memory->mapped_used)--;
                                        
                                        Seq_addlo(um.memory->unmapped_ids, (void *)(uintptr_t)id);

                                        //uint32_t new_id = map_segment(um.memory, segment->seg_length);
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

                                        seg_T new_m0 = um.memory->mapped_ids[new_id];

                                        /* Copy the instructions from segment into the new program. */
                                        for (int i = 0; i < segment->seg_length; i++) {
                                                new_m0->segments[i] = segment->segments[i];
                                        }
                                }

                                um.program_counter = um.registers[um.rc];
                                break;
                        case 13: 
                                um.ra = um_instruction << OP_CODE_LEN >> 29;
                                value = um_instruction << 7 >> 7;
                                um.registers[um.ra] = value;
                                break;
                }

                (void)value;

                /* If a new program is loaded, make sure that the program 
                 * length is changed. */
                if (op_code == 12) {
                        segment_zero = (um.memory->mapped_ids[0])->segments;
                        program_length = (um.memory->mapped_ids[0])->seg_length;
                        continue;
                }

                um.program_counter++;
        }

        fclose(fp);

        //mem_free

       

        return EXIT_SUCCESS; 
}

/* Purpose: Try to open file with given name in given mode.
 * Input:   fname -- file name
 *          mode -- mode of opening file
 *          CRE if file non-existent or error opening file.
 * Output:  Pointer to open file.
 */
// static FILE *open_or_abort(char *fname, char *mode)
// {
//         /* Assert file opened before returning pointer. */
//         FILE *fp = fopen(fname, mode);
//         assert(fp != NULL);

//         return fp;
// }

/* MEMORY FUNCTIONS */

/* Purpose: To create an instance of our memory struct
* Input: none
* Output: an instance of mem_T, our memory struct
*/
// mem_T mem_new() 
// {
//         mem_T memory = malloc(sizeof(*memory));
//         memory->mapped_used = 0;
//         memory->mapped_length = 128;
//         memory->mapped_ids = calloc(memory->mapped_length, 
//                                         sizeof(seg_T));
//         memory->unmapped_ids = Seq_new(HINT);

//         return memory;
// }

/* Purpose: To free all heap allocated memory from memory struct
* Input: mem_segments -- the memory struct containing our sequences
* Output: none
*/
// void mem_free(mem_T mem_segments) 
// {
//         uint32_t length = mem_segments->mapped_length;
        
//         /* Frees the array of words inside the memory segment. */
//         for (uint32_t i = 0; i < length; i++) {
//                 seg_T delete_segment = mem_segments->mapped_ids[i];
//                 if (delete_segment != NULL) {
//                         free(delete_segment->segments);
//                         free(delete_segment);
//                 }
//         }

//         //Seq_free(&(mem_segments->mapped_ids));
//         Seq_free(&(mem_segments->unmapped_ids));
//         free(mem_segments);
// }

/* Purpose: To map a segment of memory
* Input: mem_segments -- our memory struct containing our sequences
*        num_words -- the size of the memory segment 
* Output: none
* It is a checked runtime error to try to map more than 2^32 segments.
*/
// uint32_t map_segment(mem_T mem_segments, uint32_t num_words)
// {
//         seg_T new_segment = malloc(sizeof(*new_segment));
//         assert(new_segment != NULL);

//         uint32_t *new_seg = calloc(num_words, sizeof(new_seg));
//         assert(new_seg != NULL);

//         new_segment->segments = new_seg;
//         new_segment->seg_length = num_words;

//         uint32_t new_id;
        
//         /* Attempts to reuse an unmapped segment identifier. */
//         if (Seq_length(mem_segments->unmapped_ids) != 0) {
//                 new_id = (uint32_t)(uintptr_t)
//                          Seq_remlo(mem_segments->unmapped_ids);
//         } else {
//                 new_id = mem_segments->mapped_used;

//                 if ((new_id == mem_segments->mapped_length) &&
//                     (mem_segments->mapped_length != UINT32_MAX)) {
//                         mem_segments->mapped_length =
//                                 mem_segments->mapped_length * 2;
//                         mem_segments->mapped_ids = realloc(
//                                         mem_segments->mapped_ids,
//                                         mem_segments->mapped_length * sizeof(seg_T));
//                     }
//         }

//         mem_segments->mapped_ids[new_id] = new_segment;
//         mem_segments->mapped_used++;

//         return new_id;
// }

/* Purpose: To unmap a segment of memory
* Input: mem_segments -- our memory struct containing our sequences
*        id -- the memory segment's identifier
* Output: none
*/
// void unmap_segment(mem_T mem_segments, uint32_t id)
// {
//         seg_T delete_segment = mem_segments->mapped_ids[id];
//         //assert(delete_segment != NULL);

//         free(delete_segment->segments);
//         free(delete_segment);
        
//         mem_segments->mapped_ids[id] = NULL;
//         (mem_segments->mapped_used)--;
        
//         Seq_addlo(mem_segments->unmapped_ids, (void *)(uintptr_t)id);
// }

/* Purpose: Creates the program m[0] and adds it to mapped ids
* Input: mem_segments -- our memory struct containing our sequences
*        instructions -- the stream of instructions for the UM
* Output: none
*/
// void initialize_program(mem_T mem_segments, int program_length)
// {
//         seg_T program = malloc(sizeof(*program));
//         assert(program != NULL);
        
//         uint32_t *m0 = calloc(program_length, sizeof(m0));
//         assert(m0 != NULL);

//         program->segments = m0;
//         program->seg_length = program_length;

//         mem_segments->mapped_ids[0] = program;
//         (mem_segments->mapped_used)++;

// }

/* Purpose: loads a value from a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*                  index -- the index within the memory segment
* Output: the value stored in the memory segment
*/
// uint32_t segmented_load(mem_T mem_segments, uint32_t id, uint32_t index)
// {
//         seg_T segment = mem_segments->mapped_ids[id];
        
//         assert(segment != NULL);

//         return segment->segments[index];
// }

/* Purpose: stores a value into a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*                  index -- the index within the memory segment
*                  word -- value to store
* Output: None
*/
// void segmented_store(mem_T mem_segments, uint32_t id, uint32_t index, 
//                      uint32_t word)
// {
//         seg_T segment = mem_segments->mapped_ids[id];
//         // seg_T segment = Seq_get(mem_segments->mapped_ids, id);
//         assert(segment != NULL);

//         segment->segments[index] = word;
// }

/* Purpose: loads a new program m[0] from a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
* Output: None
* Effects: changes the running program
*/
// void load_program(mem_T mem_segments, uint32_t id)
// {
//         if (id == 0) {
//                 return;
//         }

//         seg_T segment = mem_segments->mapped_ids[id];
        
//         unmap_segment(mem_segments, 0);

//         uint32_t new_id = map_segment(mem_segments, segment->seg_length);
//         seg_T new_m0 = mem_segments->mapped_ids[new_id];

//         /* Copy the instructions from segment into the new program. */
//         for (int i = 0; i < segment->seg_length; i++) {
//                 new_m0->segments[i] = segment->segments[i];
//         }
// }

/* Purpose: gets the length of a segment in memory
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
* Output: the length of the memory segment
*/
// uint32_t segment_length(mem_T mem_segments, uint32_t id)
// {
//         // seg_T segment = Seq_get(mem_segments->mapped_ids, id);
//         seg_T segment = mem_segments->mapped_ids[id];
        
//         return segment->seg_length;
// }

/* Purpose: gets the word stored in a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*               index -- the index within the memory segment
* Output: the word stored in the memory segment
*/
// uint32_t segment_word(mem_T mem_segments, uint32_t id, int index)
// {
//         seg_T segment = mem_segments->mapped_ids[id];
        
//         return segment->segments[index];
// }


/* Purpose: takes the instruction bit and extracts the register values
* Input: instruction -- 32-bit encodings which tell the UM what to do
*        um -- our Universal Machine struct containing our memory
*        and register information
* Output: none
*/
// static inline void set_three_registers(uint32_t instruction, um_T *um)
// {
//         um->ra = instruction << 23 >>29;
//         um->rb = instruction << 26 >> 29;
//         um->rc = instruction & 0x7;
// }


#undef NUM_REGISTERS
#undef OP_CODE_LEN
#undef REGISTER_LEN
#undef VALUE_LEN 


#undef HINT
#undef INSTRUCTION_LEN
