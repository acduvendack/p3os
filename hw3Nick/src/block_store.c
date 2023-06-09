#include <stdio.h>
#include <stdint.h>
#include "bitmap.h"
#include "block_store.h"
// include more if you need

// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

typedef struct block{
    unsigned char bytes[BLOCK_SIZE_BYTES];
} block_t;

typedef struct block_store{
    bitmap_t* bitmapPtr;
    block_t blockArray[BLOCK_STORE_NUM_BLOCKS];
} block_store_t;

block_store_t *block_store_create()
{
    block_store_t* newBlock = (block_store_t*)calloc(1, sizeof(block_store_t));
    
    if (newBlock != NULL){
        newBlock->bitmapPtr = bitmap_create(BLOCK_STORE_NUM_BLOCKS);
        
        if (newBlock->bitmapPtr != NULL){
            return newBlock;
        }
    }
    
    return NULL;
}

void block_store_destroy(block_store_t *const bs)
{
    if (bs != NULL) 
    {
        if (bs->bitmapPtr != NULL){
            bitmap_destroy(bs->bitmapPtr);
        }

        free(bs);
    }
}

size_t block_store_allocate(block_store_t *const bs)
{
    if (bs == NULL){
        return SIZE_MAX;
    } else if (bs->bitmapPtr == NULL){
        return SIZE_MAX;
    }
    
    //search for free block
    size_t i = bitmap_ffz(bs->bitmapPtr);
    
    if (i == SIZE_MAX || i >= BLOCK_STORE_AVAIL_BLOCKS){
        return SIZE_MAX;
    }
    
    //mark block as in use
    bitmap_set(bs->bitmapPtr, i);
    
    // return block id
    return i;
}

bool block_store_request(block_store_t *const bs, const size_t block_id)
{
    if (bs == NULL){
        return false;
    } else if (bs->bitmapPtr == NULL){
        return false;
    }
    
    if (block_id < BLOCK_STORE_AVAIL_BLOCKS){
        if (!(bitmap_test(bs->bitmapPtr, block_id))){
            bitmap_set(bs->bitmapPtr, block_id);
            return true;
        }
    }
    
    //UNUSED(bs);
    //UNUSED(block_id);
    return false;
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
    if (bs == NULL){
    	return;
    } else if ((int)block_id < 0){
    	return;
    } else if (block_id < BLOCK_STORE_AVAIL_BLOCKS){
    	if (bitmap_test(bs->bitmapPtr, block_id)){
    		bitmap_reset(bs->bitmapPtr, block_id);
    		//free(bs->blockArray[block_id].bytes);
    		return;
    	}
    }
    
}

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
    if (bs == NULL){
        return SIZE_MAX;
    } 
    return bitmap_total_set(bs->bitmapPtr);
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
        if (bs == NULL){
        return SIZE_MAX;
    }
    return bitmap_get_bits(bs->bitmapPtr) - bitmap_total_set(bs->bitmapPtr) - 1;
}

size_t block_store_get_total_blocks()
{
    return BLOCK_STORE_AVAIL_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
     if (bs != NULL && block_id < 255 && buffer != NULL){
    memcpy(buffer, bs->blockArray[block_id].bytes, 256);
    return 256;
  }
  else return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
     if (bs != NULL && block_id < 255 && buffer != NULL){
     memcpy(bs->blockArray[block_id].bytes, buffer, 256);
    return 256;
  }
  else return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
     if (filename != NULL){
    int fd = open(filename, O_RDONLY);
    if (fd < 0)//On error, -1 is returned.
     {
      return 0;
    }
    block_store_t *bs = block_store_create(filename);
    int readOne = read(fd, bs->blockArray, BLOCK_STORE_AVAIL_BLOCKS*BLOCK_SIZE_BYTES);
    int readTwo = read(fd, bs->bitmapPtr, BLOCK_STORE_NUM_BLOCKS/8);
    if (readOne > 0 && readTwo > 0){ return bs;}
  }
  return 0;
      /*
  UNUSED(filename);
  return 0;
      */
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{ 
     if (bs == NULL || filename == NULL) {return 0;}
  int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  //int fd = open(filename, O_WRONLY);
  if (fd >= 0)
    {
      write(fd, bs->blockArray, BLOCK_STORE_AVAIL_BLOCKS*BLOCK_SIZE_BYTES);
      write(fd, bs->bitmapPtr, BLOCK_STORE_NUM_BLOCKS/8);
      close(fd);
      return (block_store_get_used_blocks(bs) * BLOCK_SIZE_BYTES);
     
    }
  //return 0;
  return BLOCK_SIZE_BYTES;
  /*
    UNUSED(bs);
    UNUSED(filename);
    return 0;
  */
}
