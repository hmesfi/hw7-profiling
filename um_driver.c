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
#include <sys/stat.h>

#define HINT 100
#define INSTRUCTION_LEN 4

static FILE *open_or_abort(char *fname, char *mode);

int main(int argc, char *argv[])
{
        if (argc != 2) {
                return EXIT_FAILURE;
        }

        FILE *fp = open_or_abort(argv[1], "rb");

        /* 
         * confirm scan of provided program successful, then get info 
         * Note: size of file divided by 4 since 32-bit word == 4 bytes,
         * so this gives us total number of 32-bit words
         */
        struct stat program_info;
        assert(stat(argv[1], &program_info) == 0); 

        /* Note: will only read up to the last complete word in file */
        const int total_words = program_info.st_size / 4;

        /* Create the instruction stream that will be used to run the UM. */
        uint32_t instruction_stream[total_words];
        
        for (int word = 0; word < total_words; word++) {
                instruction_stream[word] = 0;
                for (int lsb = 24; lsb >= 0; lsb -= 8) {
                        uint32_t piece = getc(fp);
                        instruction_stream[word] = Bitpack_newu(
                                                   instruction_stream[word],
                                                   8,
                                                   lsb,
                                                   piece);
                }
        }

        run_um(instruction_stream, total_words);

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
