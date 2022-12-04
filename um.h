/********************************************************************
 *
 *                     um.h
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *      
 *     
 *     Interface for um.h module. Contains functions
 *     that run the UM.
 ********************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <bitpack.h>
#include <seq.h>
#include "memory_segment.h"

#ifndef UM
#define UM

typedef struct um_T um_T;

/* Performs all the functions of the UM. */
void run_um(uint32_t *instructions, uint32_t program_length); 

#endif
