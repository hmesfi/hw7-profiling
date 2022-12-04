/********************************************************************
 *
 *                     memory_segment.c
 *
 *     Assignment: um
 *     Authors:  Dan Patterson (dpatte04), Helina Mesfin (hmesfi01)
 *     Date:     Nov 21, 2022
 *
 *     Purpose:
 *      
 *     Implementation for memory_segment.c module. Contains functions
 *     that manage the memory segments, both mapped and unmapped,
 *     and the definition of our struct that allows us to represent
 *     the memory. 
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <mem.h>
#include <math.h>
#include "memory_segment.h"

#define HINT 100

struct seg_T {
        uint32_t *segments; 
        int seg_length;
};

struct mem_T {
        // Seq_T mapped_ids;
        // Seq_T unmapped_ids;
        
        seg_T *mapped_ids;
        uint32_t mapped_used;
        uint32_t mapped_length;
        
        uint32_t *unmapped_ids;
        uint32_t unmapped_used;
        uint32_t unmapped_length;
};

/* Purpose: To create an instance of our memory struct
* Input: none
* Output: an instance of mem_T, our memory struct
*/
mem_T mem_new() 
{
        mem_T memory = malloc(sizeof(*memory));
        /* TODO malloc space for mapped and unmapped arrays */
        memory->mapped_used = 0;
        memory->mapped_length = 128;
        memory->mapped_ids = calloc(memory->mapped_length, 
                                        sizeof(seg_T));
        for (uint32_t i = 0; i < memory->mapped_length; i++) {
                seg_T curr = (memory->mapped_ids)[i];
                curr->segments = NULL;
                curr->seg_length = 0;
        }

        memory->unmapped_used = 0;
        memory->unmapped_length = 16;
        memory->unmapped_ids = malloc(sizeof(memory->unmapped_length) * 
                                      memory->unmapped_length);
        for (uint32_t i = 0; i < memory->unmapped_length; i++) {
                (memory->unmapped_ids)[i] = 0;
        }
        
        return memory;
}

/* Purpose: To free all heap allocated memory from memory struct
* Input: mem_segments -- the memory struct containing our sequences
* Output: none
*/
void mem_free(mem_T mem_segments) 
{
        assert(mem_segments != NULL);
        
        uint32_t length = mem_segments->mapped_length;
        
        /* Frees the array of words inside the memory segment. */
        for (uint32_t i = 0; i < length; i++) {
            if (mem_segments->mapped_ids[i] != NULL) {
                seg_T delete_segment = mem_segments->mapped_ids[i];

                free(delete_segment->segments);
                free(delete_segment);
            }
        }

        // Seq_free(&(mem_segments->mapped_ids));
        // Seq_free(&(mem_segments->unmapped_ids));
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
        assert(mem_segments->mapped_used < pow(2, 32) + 1);

        seg_T new_segment = malloc(sizeof(*new_segment));

        uint32_t *new_seg = calloc((num_words), sizeof(*new_seg));
        assert(new_seg != NULL);

        new_segment->segments = new_seg;
        new_segment->seg_length = num_words;

        uint32_t new_id = 0;
        
        fprintf(stderr, "before if statement mapping seg \n");
        /* Attempts to reuse an unmapped segment identifier. */
        if (mem_segments->unmapped_used != 0) {
                // new_id = (uint32_t)(uintptr_t)
                //          Seq_remlo(mem_segments->unmapped_ids);
                for (uint32_t i = 0; i < mem_segments->unmapped_used; i++) {
                        if (mem_segments->unmapped_ids[i] != 0) {
                                // new_id = Seq_remlo(mem_segments->unmapped_ids)
                                new_id = mem_segments->unmapped_ids[i];
                                fprintf(stderr, "(reuse) map id: %u\n", new_id);
                                mem_segments->unmapped_ids[i] = 0;
                                (mem_segments->unmapped_used)--;

                                // Seq_put(mem_segments->mapped_ids, new_id, new_segment)
                                mem_segments->mapped_ids[new_id] = new_segment;
                                (mem_segments->mapped_used)++;
 
                                return new_id;
                        }
                }

                //Seq_put(mem_segments->mapped_ids, new_id, new_segment);
        } else {
                new_id = mem_segments->mapped_used;
                fprintf(stderr, "(add hi)map id: %u\n", new_id);
                // fprintf(stderr, "mem_segments->mapped_length is: %u \n", mem_segments->mapped_length);
                if ((new_id == mem_segments->mapped_length) && 
                    (mem_segments->mapped_length != pow(2, 32))) {
                        // TODO: resize array
                        fprintf(stderr, "before resize function \n");
                        fprintf(stderr, "%p\n", (void *)mem_segments->mapped_ids);
                        resize(mem_segments, 'm');
                        fprintf(stderr, "%p\n", (void *)mem_segments->mapped_ids);
                }

                (mem_segments->mapped_ids)[new_id] = new_segment;
                (mem_segments->mapped_used)++;

                //Seq_addhi(mem_segments->mapped_ids, new_segment);
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
        assert(id < mem_segments->mapped_length);
        fprintf(stderr, "unmap id: %u\n", id);

        seg_T delete_segment = mem_segments->mapped_ids[id];
        assert(delete_segment != NULL);

        free(delete_segment->segments);
        free(delete_segment);
        
        mem_segments->mapped_ids[id] = NULL;
        (mem_segments->mapped_used)--;

        // Seq_addlo(mem_segments->unmapped_ids, (void *)(uintptr_t)id);
        if ((mem_segments->unmapped_used == mem_segments->unmapped_length) && 
            (mem_segments->unmapped_length != pow(2, 32))) {
                // TODO: resize array
                resize(mem_segments, 'u');
        }

        for (uint32_t i = 0; i < mem_segments->unmapped_used; i++) {
                if (mem_segments->unmapped_ids[i] == 0) {
                        mem_segments->unmapped_ids[i] = id;
                        (mem_segments->unmapped_used)++;
 
                        return;
                }
        }
}

/* Purpose: Creates the program m[0] and adds it to mapped ids
* Input: mem_segments -- our memory struct containing our sequences
*        instructions -- the stream of instructions for the UM
* Output: none
*/
void initalize_program(mem_T mem_segments, uint32_t *instructions, 
                                        uint32_t program_length)
{
        seg_T program = malloc(sizeof(*program));
        assert(program != NULL);
        
        uint32_t *m0 = calloc(program_length, sizeof(m0));
        assert(m0 != NULL);

        /* Copy the instructions from the stream of instructions into the 
         * program. */
        for (uint32_t i = 0; i < program_length; i++) {
                m0[i] = instructions[i];
        }

        program->segments = m0;
        program->seg_length = program_length;

        // Seq_addhi(mem_segments->mapped_ids, program);
        mem_segments->mapped_ids[0] = program;
        (mem_segments->mapped_used)++;
}

/* Purpose: loads a value from a memory segment
* Input: mem_segments -- our memory struct containing our sequences
*                  id -- the memory segment identifier
*                  index -- the index within the memory segment
* Output: the value stored in the memory segment
*/
uint32_t segmented_load(mem_T mem_segments, uint32_t id, uint32_t index)
{
        assert(id < mem_segments->mapped_length);
        fprintf(stderr, "length: %u\n", mem_segments->mapped_length);

        fprintf(stderr, "id: %u\n", id);
        seg_T segment = mem_segments->mapped_ids[id];
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
        assert(id < mem_segments->mapped_length);

        seg_T segment = mem_segments->mapped_ids[id];
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

        seg_T segment = mem_segments->mapped_ids[id];
        
        unmap_segment(mem_segments, 0);

        uint32_t new_id = map_segment(mem_segments, segment->seg_length);
        seg_T new_m0 = mem_segments->mapped_ids[new_id];

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
        assert(mem_segments->mapped_ids != NULL);

        seg_T segment = mem_segments->mapped_ids[id];
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
        seg_T segment = mem_segments->mapped_ids[id];
        return segment->segments[index];
}

/* Purpose: gets the length of the the program stored in m[0]
* Input: um -- our Universal Machine struct containing our memory
*        and register information
* Output: the length of the program
*/
uint32_t get_program_length(mem_T mem_segments)
{
        seg_T seg_zero = mem_segments->mapped_ids[0];
        return seg_zero->seg_length;
}


/* Purpose: to resize the arrays after mapping/unmapping a segment
* Input: mem_segments -- our memory struct containing our sequences
        char *array -- word containing whether its mapped or unmapped;
* Output: the length of the program
*/
void resize(mem_T mem_segments, char array_type)
{
        if (array_type == 'm') {
                mem_segments->mapped_length = 
                                mem_segments->mapped_length * 2;
                mem_segments->mapped_ids = realloc(
                                mem_segments->mapped_ids, 
                                mem_segments->mapped_length * sizeof(seg_T));
        } else if (array_type == 'u') {
                mem_segments->mapped_length = 
                                mem_segments->mapped_length * 2;
                mem_segments->unmapped_ids = realloc(
                                mem_segments->unmapped_ids, 
                                mem_segments->unmapped_length * sizeof(uint32_t));
        }
}

#undef HINT
