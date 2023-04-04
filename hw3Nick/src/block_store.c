#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

//block structure that uses unsigned char because it is 8 bits
typedef struct block{
    unsigned char bytes[BLOCK_SIZE_BYTES];
} block_t;

//block store structure that has array of blocks and bitmap to keep track of used blocks
typedef struct block_store{
    bitmap_t* bitmapPtr;
    block_t blockArray[BLOCK_STORE_NUM_BLOCKS];
} block_store_t;

//Allocates the memory for a new block_store structure. If successful
//the bitmap pointer is allocated with a bitmap using bitmap_create
block_store_t *block_store_create()
{
    //allocate block_store
    block_store_t* newBlock = (block_store_t*)calloc(1, sizeof(block_store_t));
    
    //check for error
    if (newBlock != NULL){
        //allocate bitmap
        newBlock->bitmapPtr = bitmap_create(BLOCK_STORE_NUM_BLOCKS);
        
        //check for error
        if (newBlock->bitmapPtr != NULL){
            return newBlock;
        }
    }
    
    //return null if error
    return NULL;
}

//deallocates a block_store and the bitmap pointer
void block_store_destroy(block_store_t *const bs)
{
    //checks if passed null parameter
    if (bs != NULL) 
    {
        //checks if bitmap pointer is valid
        if (bs->bitmapPtr != NULL){
            //deallocates bitmap pointer
            bitmap_destroy(bs->bitmapPtr);
        }

        //deallocates block_store
        free(bs);
    }
}

//allocates the first free block in a block_store
size_t block_store_allocate(block_store_t *const bs)
{
    //checks for valid parameters
    if (bs == NULL){
        return SIZE_MAX;
    } else if (bs->bitmapPtr == NULL){
        return SIZE_MAX;
    }
    
    //search for free block
    size_t i = bitmap_ffz(bs->bitmapPtr);
    
    //return SIZE_MAX if over allocated or error during bitmap_ffz
    if (i == SIZE_MAX || i >= BLOCK_STORE_AVAIL_BLOCKS){
        return SIZE_MAX;
    }
    
    //mark block as in use
    bitmap_set(bs->bitmapPtr, i);
    
    // return block id
    return i;
}

//request for a specific block id to be allocated in a block_store
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    //checks for valid parameters
    if (bs == NULL){
        return false;
    } else if (bs->bitmapPtr == NULL){
        return false;
    }
    
    //checks that block_id is within range of blocks
    if (block_id < BLOCK_STORE_AVAIL_BLOCKS){
        //checks if block is not in use
        if (!(bitmap_test(bs->bitmapPtr, block_id))){
            //sets block if not in use
            bitmap_set(bs->bitmapPtr, block_id);
            return true;
        }
    }
    
    //return false if block could not be used
    return false;
}

//releases the data stored in a block storage.
void block_store_release(block_store_t *const bs, const size_t block_id)
{
    //param check
    if (bs == NULL){
    	return;
    } else if ((int)block_id < 0){
    	return;
    } else if (block_id < BLOCK_STORE_AVAIL_BLOCKS){
        //resets a bit in the block storage
    	if (bitmap_test(bs->bitmapPtr, block_id)){
    		bitmap_reset(bs->bitmapPtr, block_id);
    		//free(bs->blockArray[block_id].bytes);
    		return;
    	}
    }
    
}


//checks how many bits are used
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    //param check
    if (bs == NULL){
        return SIZE_MAX;
    } 
    //returns the bits set
    return bitmap_total_set(bs->bitmapPtr);
}

//Returns the number of free bits
size_t block_store_get_free_blocks(const block_store_t *const bs)
{
    //param check
        if (bs == NULL){
        return SIZE_MAX;
    }
    //returns free bits
    return bitmap_get_bits(bs->bitmapPtr) - bitmap_total_set(bs->bitmapPtr) - 1;
}

//returns the total number of free blocks
size_t block_store_get_total_blocks()
{
    //the number of blocks to return
    return BLOCK_STORE_AVAIL_BLOCKS;
}

//writes a block store to a buffer
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
    //checks for errors
    if (bs != NULL && block_id < 255 && buffer != NULL){
        //writes to buffer
        memcpy(buffer, bs->blockArray[block_id].bytes, 256);
        return 256;
    }
    else return 0;
}

//writes a buffer to a block store
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
    //checks for errors
    if (bs != NULL && block_id < 255 && buffer != NULL){
        //writes to block store
        memcpy(bs->blockArray[block_id].bytes, buffer, 256);
        return 256;
    }
    else return 0;
}

//writes file info to a block store
block_store_t *block_store_deserialize(const char *const filename)
{
	int fd = 0;

	//error check
	if (filename == NULL)
	{
        	return 0;
	}

	//open file
	fd = open(filename, O_RDONLY);
	
	//return 0 if error
	if (fd < 0)
	{
        	return 0;
	}
	
	//initialize blockstore
	block_store_t *bs = NULL;
	bs = block_store_create(filename);

	for(int i = 0; i <= BLOCK_STORE_AVAIL_BLOCKS; i++) //iterate through blockArray locations
	{
		if (block_store_request(bs, i) == true) //request for block id "i" to be allocated in b
		{
        		if (read(fd, bs->blockArray[i].bytes, BLOCK_SIZE_BYTES) < 0) //If error return 0
          		{
				return 0;
			}
		}
	}
	close(fd); //close file
	return bs; //return block_store_t struct
}

//writes block store information to a file
size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
    //initialize variables
    int fd = 0;
    size_t numOfBytes = 0;
    
    //error check
    if (bs == NULL || filename == NULL){
        return 0;
    }
    
    //open file
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO | S_IRWXG);
    
    //return error of file
    if (fd == -1){
        return errno;
    }
    
    //write bs info to a file and return size written
    for(int i = 0; i <= BLOCK_STORE_AVAIL_BLOCKS; i++){
        size_t writeBytes = write(fd, bs->blockArray[i].bytes, BLOCK_SIZE_BYTES);
        
        numOfBytes = numOfBytes + writeBytes;
    }
    
    //close file
    close(fd);
    return numOfBytes;
}
