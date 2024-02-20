# mymalloc

This project was created by:

1. Rachit Gupta: rg1091
2. Vian Miranda: vsm35

## Representing a chunk using struct

Our design is based on creating a `chunk` struct, which consists of a `chunkheader` for the header and a `chunkpayload` for the payload. `chunkheader` keeps track of the 2 properties of our payload: `size` and `is_allocated`, which indicates if the chunk is free or taken. We found this way of representing a chunk simplifies how we create our helper methods that deal with chunk size, allocation status, and getting the payload and next chunk because it makes the code clearer to write and involves less arithmetic than manipulating each byte.

## Mymalloc design

`mymalloc()` returns a pointer to the beginning of a chunk of memory (the payload) dependent on the number of bytes of memory the user requests. 

When `mymalloc()` is used, we first check if the size requested is `!= 0` (otherwise return an error message and `NULL`). Since size is of type `size_t`, we know a negative amount of bytes is not allowed; however, this may lead to a buffer underflow, which we must also deal with. We can check that this isn't happening by ensuring the max size allocatable is less than or equal to `(MEMLENGTH - HEADERSIZE)`. We ensure that the size requested is rounded up to the nearest multiple of 8 to stay consistent with 8-byte alignment. On the first call of malloc, when our chunk size is 0 and is free, we initialize a new chunk to the size of our total memory and mark it as free for future mallocing.

To allocate memory, we traverse through the chunks until we find one that is free and has enough size for `neededSize`, which is the `size requested + the size of the header`. If we find a chunk that meets these requirements, we allocate the chunk of `neededSize` and mark it is allocated. We also ensure that if there is enough of remaining space to allocate a new chunk from the original size, we split the current chunk into two chuncks, with the second chunk's size being the remaining chunk space. Otherwise, if the remaining size is not enough for a new chunk, it is absorbed by the chunk being allocated currently. Finally, we return a pointer to the payload for the memory space requested.

If we traverse through all the chunks and do not find one, it is enough to conclude that there was insufficient memory and return an error. This is because we had originally initialized the chunk size to the max memory size, so getting to this point would indicate the user requested even more memory.


## Myfree design

`myfree()` takes in a pointer to a chunk and marks it as free.

The first thing we check is if the pointer is `NULL`, which should not be possible to free, and return an error message.

We traverse through the chunks the same way as we did in `mymalloc()`, so until the pointer to the current chunk's payload matches the pointer to the chunk the user requested. In this case, we want to check if we are freeing an object that could have already been freed - if its allocation status is marked as free, we return an error message. Otherwise if the chunk was not already free, we mark it as free. 

We want to automatically coalesce provided chunk with the next chunk if the next chunk is free to simplify using `mymalloc()` again. We also want to coalesce when the previous chunk is free. 

1. Coalescing with next chunk:
  We created a helper function called `coalesce()` that takes in a pointer to the requested chunk's header. We check if the current chunk and next available chunk are both free, and adjust the chunk size of the current chunk to the size of current and next chunk combined. We then ensure that the chunksize of what the next chunk was to be reset to 0. This function returns 1 to indicate the next chunk was properly coalesced with the current chunk, or 0 if there were never adjacent chunks that could be coalesced. 

2. Coalescing with previous chunk:
  This occurs after we have already tried to coalesce the current chunk with the next chunk. In this case, however, we want to also check if we can continue to coalesce with the previous chunk to be able to create larger contiguous free chunks. We create a previous pointer that is initially `NULL`, but eventually becomes a pointer to the current chunk after coalescing with the next chunk. In the case that the previous pointer is not `NULL` and we coalesced with the previous chunk, we update the pointer to the current chunk to the "previous" pointer.

Finally, if we reach the end of memory without finding the pointer to the requested chunk, we throw an error message indicating that the requested chunk was never allocated in the first place or we are not freeing at the start of the requested memory chunk.


## Testing

We include 2 files, `memtest.c` and `memgrind.c` - both do not require any arguments. In `memtest.c`, we aim to perform black box testing to see if `mymalloc()` and `myfree()` work as expected and throw the right errors at the right time. In `memgrind.c`, we perform stress tests on `mymalloc()` and `myfree()` to see how they compare against the real malloc and free.


## Correctness testing (memtest)

1. Number of incorrect bytes test -
 We have an array of objects. We fill memory with objects using `mymalloc()` and fill each object with distinct bytes. We then check how many incorrect bytes there are, and free the objects.

2. Allocating and initializing memory -
  We create an object array and `mymalloc()` each object 32 times. We then initialize each object to the iteration number (`i`)

3. Pointer overlap test -
  We check if the value stored at the beginning of each allocated block matches the expected value (`i`). If it doesn't, it indicates that there is overlapping memory.

4. Mallocing more memory than available -
  At this point, memory is currently full. We include another malloc statement to show how doing so would produce a max memory error.

5. Freeing in middle of chunk test -
  We attempt to free 4 bytes ahead of the start of one of the allocated chunks, this should throw error for bad pointer.
  
6. Free all allocated memory -
  We free all memory to prepare for the next test

7. Double free test -
  We try to free a pointer that is already freed, so we should get a double free pointer error.

8. Mallocing 0 bytes -
  We attempt to malloc 0 bytes, this throws an error because we need `>0` bytes to malloc

9. Mallocing negative bytes -
  We attempt to malloc -1 bytes, this throws a buffer underflow error because we need `<=(MEMLENGTH - HEADER_SIZE)` bytes to malloc

## Performance testing (memgrind)
We perform 5 tests in memgrind. We created a function called runTest() that takes in the test as a parameter and calculates the average time to complete the test after 50 runs.

1. Test 1
  We consecutively `mymalloc()` and `myfree()` a 1-byte object 120 times. We do this to check if memory is correctly being deallocated after repetion.
2. Test 2
  We use `mymalloc()` to store pointers to 120 1-byte objects in an array, and then use `myfree()` to deallocate the chunks. This is done to test the performance of adding the pointers to an array and freeing them after access.
3. Test 3
  We create an array of 120 pointers and randomly `mymalloc()` a 1-byte object or deallocate a previously allocated object (if any) until we have allocated 120 times. Finally, we deallocate any remaining objects. The purpose of this test is to see how efficiently `mymalloc()` and `myfree()` handle randomness. 
4. Test 4
  Repeatedly allocate and deallocate 2 2032-byte memory blocks 50 times. This is done to assess how we handle larger memory allocations and deallocations.
5. Test 5
  Repeatedly allocate and deallocate 4 different types of memory blocks(`int`, `char`, `double`, `void`) 25 times. This tests how efficiently we can manage the memory for various objects.


## Conclusion

We found that our version of malloc and free is definitely slower than the original. This makes sense because unlike the real malloc and free which prioritizes efficiency and assumes that the user understands how to properly use them, we incorporate a lot more error checking which takes up more space and time.