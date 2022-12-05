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
#include <bitpack.h>
#include <seq.h>

#define HINT 100
#define INSTRUCTION_LEN 4

#define NUM_REGISTERS 8
#define OP_CODE_LEN 4
#define REGISTER_LEN 3
#define VALUE_LEN 25
#define HINT 100

typedef struct mem_T {
        Seq_T mapped_ids;
        Seq_T unmapped_ids;
} *mem_T;

typedef struct seg_T {
        uint32_t *segments; 
        int seg_length;
} *seg_T;

typedef struct um_T {
        mem_T memory;
        uint32_t registers[NUM_REGISTERS];
        uint32_t program_counter;
        unsigned ra, rb, rc;
} um_T;

static FILE *open_or_abort(char *fname, char *mode);

/* Performs all the functions of the UM. */
void run_um(Seq_T instructions); 

/* Memory Segment functions */
/* Creates a memory instance. */
mem_T mem_new();
/* Frees a memory instance. */
void mem_free(mem_T mem_segments);
/* Maps a memory segment. */
uint32_t map_segment(mem_T mem_segments, uint32_t num_words);
/* Unmaps a memory segment. */
void unmap_segment(mem_T mem_segments, uint32_t identifier);
/* Creates the program m[0] and stores it in memory. */
void initalize_program(mem_T mem_segments, Seq_T instructions);
/* Retrieves a word from memory. */
uint32_t segmented_load(mem_T mem_segments, uint32_t id, uint32_t index);
/* Stores a word from memory. */
void segmented_store(mem_T mem_segments, uint32_t id, uint32_t index, 
                     uint32_t word);
/* Creates the program m[0] using a segment stored in memory. */
void load_program(mem_T mem_segments, uint32_t id);
/* Gets the length of a segment in memory. */
uint32_t segment_length(mem_T mem_segments, uint32_t id);
/* Gets a word from a segment in memory. */
uint32_t segment_word(mem_T mem_segments, uint32_t id, int index);


int main(int argc, char *argv[])
{
        if (argc != 2) {
                return EXIT_FAILURE;
        }

        FILE *fp = open_or_abort(argv[1], "rb");

        /* Create the instruction stream that will be used to run the UM. */
        Seq_T instruction_stream = Seq_new(HINT);
        
        int c = fgetc(fp);

        while (c != EOF) {
                uint32_t um_instruction = 0;

                for (int i = INSTRUCTION_LEN - 1; i >= 0; i--) {
                        um_instruction = Bitpack_newu(um_instruction,
                                                      INSTRUCTION_LEN * 2,
                                                      i * (INSTRUCTION_LEN * 2),
                                                      c);
                        c = fgetc(fp);
                }

                Seq_addhi(instruction_stream, 
                          (void *)(uintptr_t) um_instruction);
        }

        run_um(instruction_stream);

        Seq_free(&instruction_stream);
        fclose(fp);

        return EXIT_SUCCESS; 
}

/* Purpose: Try to open file with given name in given mode.
 * Input:   fname -- file name
 *          mode -- mode of opening file
 *          CRE if file non-existent or error opening file.
 * Output:  Pointer to open file.
 */
static FILE *open_or_abort(char *fname, char *mode)
{
        /* Assert file opened before returning pointer. */
        FILE *fp = fopen(fname, mode);
        assert(fp != NULL);

        return fp;
}

/* MEMORY FUNCTIONS */

/* Purpose: To create an instance of our memory struct
* Input: none
* Output: an instance of mem_T, our memory struct
*/
mem_T mem_new() 
{
        mem_T memory = malloc(sizeof(*memory));
        memory->mapped_ids = Seq_new(HINT);
        memory->unmapped_ids = Seq_new(HINT);

        return memory;
}

/* Purpose: To free all heap allocated memory from memory struct
* Input: mem_segments -- the memory struct containing our sequences
* Output: none
*/
void mem_free(mem_T mem_segments) 
{
        uint32_t length = Seq_length(mem_segments->mapped_ids);
        
        /* Frees the array of words inside the memory segment. */
        for (uint32_t i = 0; i < length; i++) {
            if (Seq_get(mem_segments->mapped_ids, i) != NULL) {
                seg_T delete_segment = Seq_get(mem_segments->mapped_ids, i);

                free(delete_segment->segments);
                free(delete_segment);
            }
        }

        Seq_free(&(mem_segments->mapped_ids));
        Seq_free(&(mem_segments->unmapped_ids));
        free(mem_segments);
}

/* Purpose: To map a segment of memory
* Input: mem_segments -- our memory struct containing our sequences
*        num_words -- the size of the memory segment 
* Output: none
* It is a checked runtime error to try to map more than 2^32 segments.
*/
uint32_t map_segment(mem_T mem_segments, uint32_t num_words)
{
        seg_T new_segment = malloc(sizeof(*new_segment));
        assert(new_segment != NULL);

        uint32_t *new_seg = calloc((num_words), sizeof(new_seg));
        assert(new_seg != NULL);

        new_segment->segments = new_seg;
        new_segment->seg_length = num_words;

        uint32_t new_id;
        
        /* Attempts to reuse an unmapped segment identifier. */
        if (Seq_length(mem_segments->unmapped_ids) != 0) {
                new_id = (uint32_t)(uintptr_t)
                         Seq_remlo(mem_segments->unmapped_ids);

                Seq_put(mem_segments->mapped_ids, new_id, new_segment);
        } else {
                new_id = Seq_length(mem_segments->mapped_ids);

                Seq_addhi(mem_segments->mapped_ids, new_segment);
        }

        return new_id;
}

/* Purpose: To unmap a segment of memory
* Input: mem_segments -- our memory struct containing our sequences
*        id -- the memory segment's identifier
* Output: none
*/
void unmap_segment(mem_T mem_segments, uint32_t id)
{
        seg_T delete_segment = Seq_get(mem_segments->mapped_ids, id);
        assert(delete_segment != NULL);

        free(delete_segment->segments);
        free(delete_segment);
        
        Seq_put(mem_segments->mapped_ids, id, NULL);
        Seq_addlo(mem_segments->unmapped_ids, (void *)(uintptr_t)id);
}

/* Purpose: Creates the program m[0] and adds it to mapped ids
* Input: mem_segments -- our memory struct containing our sequences
*        instructions -- the stream of instructions for the UM
* Output: none
*/
void initalize_program(mem_T mem_segments, Seq_T instructions)
{
        seg_T program = malloc(sizeof(*program));
        assert(program != NULL);
        
        uint32_t *m0 = calloc(Seq_length(instructions), sizeof(m0));
        assert(m0 != NULL);


        /* Copy the instructions from the stream of instructions into the 
         * program. */
        for (int i = 0; i < Seq_length(instructions); i++) {
                m0[i] = (uint32_t)(uintptr_t)Seq_get(instructions, i);
        }

        program->segments = m0;
        program->seg_length = Seq_length(instructions);

        Seq_addhi(mem_segments->mapped_ids, program);
}

/* Purpose: loads a value from a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*                  index -- the index within the memory segment
* Output: the value stored in the memory segment
*/
uint32_t segmented_load(mem_T mem_segments, uint32_t id, uint32_t index)
{
        seg_T segment = Seq_get(mem_segments->mapped_ids, id);
        assert(segment != NULL);

        return segment->segments[index];
}

/* Purpose: stores a value into a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*                  index -- the index within the memory segment
*                  word -- value to store
* Output: None
*/
void segmented_store(mem_T mem_segments, uint32_t id, uint32_t index, 
                     uint32_t word)
{
        seg_T segment = Seq_get(mem_segments->mapped_ids, id);
        assert(segment != NULL);

        segment->segments[index] = word;
}

/* Purpose: loads a new program m[0] from a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
* Output: None
* Effects: changes the running program
*/
void load_program(mem_T mem_segments, uint32_t id)
{
        if (id == 0) {
                return;
        }

        seg_T segment = Seq_get(mem_segments->mapped_ids, id);
        
        unmap_segment(mem_segments, 0);

        uint32_t new_id = map_segment(mem_segments, segment->seg_length);
        seg_T new_m0 = Seq_get(mem_segments->mapped_ids, new_id);

        /* Copy the instructions from segment into the new program. */
        for (int i = 0; i < segment->seg_length; i++) {
                new_m0->segments[i] = segment->segments[i];
        }
}

/* Purpose: gets the length of a segment in memory
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
* Output: the length of the memory segment
*/
uint32_t segment_length(mem_T mem_segments, uint32_t id)
{
        seg_T segment = Seq_get(mem_segments->mapped_ids, id);
        return segment->seg_length;
}

/* Purpose: gets the word stored in a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*               index -- the index within the memory segment
* Output: the word stored in the memory segment
*/
uint32_t segment_word(mem_T mem_segments, uint32_t id, int index)
{
        seg_T segment = Seq_get(mem_segments->mapped_ids, id);
        return segment->segments[index];
}

/* UM FUNCTIONS */

/* Purpose: To create an instance of the um_T struct (the Machine)
*           in its initial state
* Input: instructions -- Seq_T stream of instructions for the UM
*        
* Output: an instance of the um
*/
static inline um_T initalize_um(Seq_T instructions)
{
        um_T um;

        um.memory = mem_new();

        initalize_program(um.memory, instructions);

        for (int i = 0; i < NUM_REGISTERS; i++) {
                um.registers[i] = 0;
        }

        um.program_counter = 0;

        return um;
} 

/* Purpose: takes the instruction bit and extracts the register values
* Input: instruction -- 32-bit encodings which tell the UM what to do
*        um -- our Universal Machine struct containing our memory
*        and register information
* Output: none
*/
static inline void set_three_registers(uint32_t instruction, um_T *um)
{
        um->ra = instruction << 23 >>29;
        um->rb = instruction << 26 >> 29; //Bitpack_getu(instruction, REGISTER_LEN, REGISTER_LEN);
        um->rc = instruction & 0x7; //Bitpack_getu(instruction, REGISTER_LEN, 0);
}

/* Purpose: specifically for opcode 13, takes the instruction bit and 
*           extracts specifically register A
* Input: instruction -- 32-bit encodings which tell the UM what to do
*        um -- our Universal Machine struct containing our memory
*        and register information
* Output: none
*/
static inline uint32_t set_one_register(uint32_t instruction, um_T *um)
{
        um->ra = instruction << OP_CODE_LEN >> 29; //Bitpack_getu(instruction, REGISTER_LEN, VALUE_LEN);
        return instruction << 7 >> 7; //Bitpack_getu(instruction, VALUE_LEN, 0);
}

/* Purpose: getter function that returns the opcode by extracting
*           the four most significant bits from the instruction word
* Input:    uint32_t instruction -- 32 bit word 
*        
* Output:   returns the opcode
*/
static inline uint32_t get_opcode(uint32_t instruction)
{
        return instruction >> 28; //Bitpack_getu(instruction, OP_CODE_LEN, 28);
}


/* Purpose: runs whatever instruction is given to the UM
*           
* Input:   um -- our Universal Machine struct containing our memory
*          and register information
*          uint32_t instruction -- 32 bit word 
*          uint32_t op_code -- contains command to be run     
* Output:  none
*/
static inline void execute_command(um_T *um, uint32_t instruction, 
                                   uint32_t op_code)
{
        uint32_t value;

        switch (op_code)
        {
                case 0: 
                        set_three_registers(instruction, um);
                        if (um->registers[um->rc] != 0) {
                                um->registers[um->ra] = um->registers[um->rb];
                        }
                        break;
                case 1: 
                        set_three_registers(instruction, um);
                        um->registers[um->ra] = segmented_load(um->memory,
                                                um->registers[um->rb], 
                                                um->registers[um->rc]);
                        break;
                case 2: 
                        set_three_registers(instruction, um);
                        segmented_store(um->memory, um->registers[um->ra], 
                        um->registers[um->rb], um->registers[um->rc]);
                        break;
                case 3: 
                        set_three_registers(instruction, um);
                        um->registers[um->ra] = um->registers[um->rb] + um->registers[um->rc]; 
                        break;
                case 4:
                        set_three_registers(instruction, um);
                        um->registers[um->ra] = um->registers[um->rb] * um->registers[um->rc];
                        break;
                case 5: 
                        set_three_registers(instruction, um);
                        um->registers[um->ra] = um->registers[um->rb] / um->registers[um->rc];
                        break;
                case 6: 
                        set_three_registers(instruction, um);
                        uint32_t rb_value = um->registers[um->rb];
                        uint32_t rc_value = um->registers[um->rc];
                        um->registers[um->ra] = ~(rb_value & rc_value);
                        break;
                case 8: 
                        set_three_registers(instruction, um);
                        um->registers[um->rb] = map_segment(um->memory, um->registers[um->rc]);
                        break;
                case 9: 
                        set_three_registers(instruction, um);
                        unmap_segment(um->memory, um->registers[um->rc]);
                        break;
                case 10: 
                        set_three_registers(instruction, um);
                        putchar(um->registers[um->rc]);
                        break;
                case 11: 
                        set_three_registers(instruction, um);
                        um->registers[um->rc] = getchar();
                        break;
                case 12: 
                        set_three_registers(instruction, um);
                        load_program(um->memory, um->registers[um->rb]);
                        um->program_counter = um->registers[um->rc];
                        break;
                case 13: 
                        value = set_one_register(instruction, um);
                        um->registers[um->ra] = value;
                        break;
        }

        (void)value;
}

/* Purpose: gets the length of the the program stored in m[0]
* Input: um -- our Universal Machine struct containing our memory
*        and register information
* Output: the length of the program
*/
static inline uint32_t get_program_length(um_T *um)
{
        return segment_length(um->memory, 0);
}


/* Purpose: runs our UM program
* Input: um -- our Universal Machine struct containing our memory
*        and register information
* Output: none
*/
void run_um(Seq_T instructions)
{
        uint32_t um_instruction;

        um_T um = initalize_um(instructions);
        uint32_t program_length = get_program_length(&um);

        /* UM will keep executing commands until it reaches the end of m[0]. */
        while (um.program_counter < program_length) {

                um_instruction = segment_word(um.memory, 0, um.program_counter);
                uint32_t op_code = um_instruction >> 28;
                //get_opcode(um_instruction);

                /* Ensures that the instruction is valid. */
                assert(op_code < 14);

                if (op_code == 7) {
                        set_three_registers(um_instruction, &um);
                        mem_free(um.memory);
                        return;              
                }

                execute_command(&um, um_instruction, op_code);

                /* If a new program is loaded, make sure that the program 
                 * length is changed. */
                if (op_code == 12) {
                        program_length = get_program_length(&um);
                        continue;
                }

                um.program_counter++;
        }      
} 

#undef NUM_REGISTERS
#undef OP_CODE_LEN
#undef REGISTER_LEN
#undef VALUE_LEN 


#undef HINT
#undef INSTRUCTION_LEN
