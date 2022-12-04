/********************************************************************
 *
 *                     io.c
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *     
 *     Implementation for IO module. Contains functions necessary
 *     to utilize the I/O devices in the UM machine
 *     that perform the input and output instructions.
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>


/* Purpose: To return a byte (within ASCII range) 
*           that has been parsed through stdin
* Input:    none
*        
* Output: a uint32_t byte from stdin
*/
int input()
{
    int c = getchar();

    if (c == EOF) {
        c = ~(uint32_t)0;
    }

    return c;
}

/* Purpose: outputs a byte from a register within ASCII range
* Input:   a uint32_t byte (that will be outputted)
*        
* Output:  none
*/
void output(int op)
{
        putchar(op);
}
