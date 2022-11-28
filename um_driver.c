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
#include "um.h"

#define HINT 100
#define INSTRUCTION_LEN 4

static FILE *open_or_abort(char *fname, char *mode);

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

#undef HINT
#undef INSTRUCTION_LEN
