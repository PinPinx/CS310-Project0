#include <stdio.h> //needed for size_t
#include <unistd.h> //needed for sbrk
#include <assert.h> //For asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata { //in Headers and Footers
	size_t size;
    bool freeBlock; //True if block is free, else False
} metadata_t;

 /* freelist maintains all the blocks which are not in use;
 freelist is kept always sorted to improve the efficiency of coalescing */

static metadata_t* freelist = NULL;

void* dmalloc(size_t numbytes) {
	if(freelist == NULL) { //Initialize through sbrk call first time
		if(!dmalloc_init())
			return NULL;
	}

	assert(numbytes > 0);
    
    /* Your code goes here */
    size_t sizeHF = (2 * METADATA_T_ALIGNED); //sizeHF is size of header and footer together
    size_t size_HBF = ((ALIGN(numbytes)+ sizeHF); //size of Header+Block+Footer (of block to add)
    void* pointerToMallocedBlock;
    
    metadata_t* current = freelist; //start at beginning
    metadata_t* block1_H; //Block1: to be filled with data
    metadata_t* block1_F;
    metadata_t* block2_H; //Block2: to remain free (after split)
    metadata_t* block2_F;
    
    while(current < epilogue){ //go to the end of the heap
        if (current->freeBlock == false || current->size < size_HBF)
        //if (block not free) or (not enough space in current block), then move to the next block
        {
            current = (metadata_t*) ((void*) current + current->size + sizeHF);
        }
        else{
            
            //for first block to be filled
            block1_H = current;
            block1_H->freeBlock = false;
            block1_H->size = ALIGN(numbytes);
            block1_F = (metadata_t*) ((void*) block1_H - METADATA_T_ALIGNED); ///+_
            block1_F->freeBlock = false;
            block1_F->size = block1_H->size;
            
            ///IF IT FITS EXACTLY THEN WHAT
            ////////Make sure that there is no case where it seems to fit but the header and footer may not fit
            
            //for second block after split which remians free
            block2_H = (metadata_t*) ((void*) block1_H + size_HBF);
            block2_H->freeBlock = true;
            block2_H->size = block1_H - size_HBF;
            block2_F = (metadata_t*) ((void*) METADATA_T_ALIGNED + block2_H + block2_H->size);
            block2_F->freeBlock = true;
            block2_F->size = block2_H->size;
            
            pointerToMallocedBlock = (metadata_t*) ((void*) block1_H + METADATA_T_ALIGNED);
            
            return pointerToMallocedBlock;
        }
    }
	return NULL;
}

void dfree(void* ptr) {
	/* Your free and coalescing code goes here...*/
    
    size_t sizeHF = (2 * METADATA_T_ALIGNED); //sizeHF is size of header and footer together

    metadata_t* blocktofree_H;
    metadata_t* blocktofree_F;
    metadata_t* next_H;
    metadata_t* prev_F;
    metadata_t* new_H; //changes with coalescing with prev, else new_H = blocktofree_H
    
    blocktofree_H = (metadata_t*) ((void*) ptr - METADATA_T_ALIGNED);
    blocktofree_F = (metadata_t*) ((void*) ptr + blocktofree_H->size);
    next_H = (metadata_t*) ((void*) blocktofree_F + METADATA_T_ALIGNED);
    prev_F = (metadata_t*) ((void*) blocktofree_H - METADATA_T_ALIGNED);
    blocktofree_H->freeBlock = true;
    blocktofree_F->freeBlock= true;
    new_H = blocktofree_H;
    
    //Checks for coalescence with prev and next blocks:
    //if prev block exists and is free
    if((prev_F > freelist) && (prev_F->freeBlock == true)){
        prev_F = (metadata_t*) ((void*) prev_F - prevF->size - METADATA_T_ALIGNED);
        prev_F->freeBlock = true;
        prev_F->size = (prev_F->size + blocktofree_H->size + sizeHF); ///could this be 2*(sizeHF)?????
        new_H = prev_F;
        blocktofree_F->size = new_H->size;
    }
    //if next block exists and is free
    if((next_H < epilogue) && (next_H->freeBlock == true)){
        next_H = (metadata_t*) ((void*) next_H + next_H->size + METADATA_T_ALIGNED);
        next_H->isfree = true;
        next_H->size = (next_H->size + new_H->size + sizeHF);//could this be 2*(sizeHF)?
        new_H->size = next_H->size;
    }
}

bool dmalloc_init() {
	size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
    freelist = (metadata_t*) sbrk(max_bytes);
	/* Q: Why casting is used? i.e., why (void*)-1? */
    
	if (freelist == (void *)-1)
		return false;
    
	freelist->size = max_bytes - sizeHF; // = size of space avail
    freelist->freeBlock = true;

    epilogue = (metadata_t*) ((void*) freelist + freelist->size + METADATA_T_ALIGNED);
    epilogue->size = freelist->size;
    epilogue->freeBlock = false;
    
	return true;
}