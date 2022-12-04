/********************************************************************
 *
 *                     memory_segment.h
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *      
 *     Interface for memory_segment.h module. Contains functions
 *     that manage the memory segments, both mapped and unmapped,
 *     the memory. 
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <seq.h>

#ifndef MEMORY_SEGMENTS
#define MEMORY_SEGMENTS

typedef struct mem_T *mem_T;
typedef struct seg_T *seg_T;

/* Creates a memory instance. */
mem_T mem_new();
/* Frees a memory instance. */
void mem_free(mem_T mem_segments);
/* Maps a memory segment. */
uint32_t map_segment(mem_T mem_segments, uint32_t num_words);
/* Unmaps a memory segment. */
void unmap_segment(mem_T mem_segments, uint32_t identifier);
/* Creates the program m[0] and stores it in memory. */
void initalize_program(mem_T mem_segments, uint32_t *instructions,
                                        uint32_t program_length);
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
/* Retrieves program length*/
uint32_t get_program_length(mem_T mem_segments);
/* Resizes the mapped and unmapped arrays after mapping*/
void resize(mem_T mem_segments, char array_type);

#endif
