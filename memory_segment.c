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

struct mem_T {
        Seq_T mapped_ids;
        Seq_T unmapped_ids;
};

struct seg_T {
        uint32_t *segments; 
        int seg_length;
};

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

#undef HINT
