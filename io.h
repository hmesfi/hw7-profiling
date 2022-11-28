/********************************************************************
 *
 *                     io.h
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *     
 *     Interface for IO module. Contains functions necessary
 *     to utilize the I/O devices in the UM machine
 *     that perform both input and output instructions.
 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#ifndef io
#define io

/* Returns input from stdin. */
int input();

/* Prints a character to stdout. */
void output(int op);

#endif
