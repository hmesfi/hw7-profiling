/********************************************************************
 *
 *                     um.c
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *     
 *     Implementation for um module. Contains the struct definition
 *     for our Universal Machine and the functions necessary to 
 *     perform each instruction.
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <bitpack.h>
#include "memory_segment.h"
#include "um.h"
#include <seq.h>

#define NUM_REGISTERS 8
#define OP_CODE_LEN 4
#define REGISTER_LEN 3
#define VALUE_LEN 25

struct um_T {
        mem_T memory;
        uint32_t registers[NUM_REGISTERS];
        uint32_t program_counter;
        unsigned ra, rb, rc;
};

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
