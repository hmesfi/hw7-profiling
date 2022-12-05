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

#ifndef UM
#define UM

typedef struct um_T um_T;
typedef struct mem_T *mem_T;
typedef struct seg_T *seg_T;

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

#endif
