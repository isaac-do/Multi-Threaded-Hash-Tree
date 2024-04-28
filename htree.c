#include <stdio.h>     
#include <stdlib.h>   
#include <stdint.h>  
#include <inttypes.h>  
#include <errno.h>     // for EINTR
#include <fcntl.h>     
#include <unistd.h>    
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#include "common_threads.h"
#include "common.h"

// Print out the usage of the program and exit.
void Usage(char*);
uint32_t jenkins_one_at_a_time_hash(const uint8_t*, uint64_t);
void* calculateHash(void*);
void* tree(void* arg);

// block size
#define BSIZE 4096
#define MAX_HASH_SIZE 100

struct threadArg
{ // this can have a better name tbh idk what to call it
    int num_threads; // number of threads
    int thread_id; // thread id
    uint8_t* file_data; // point to the file data in memory
    size_t size; // size of thread
};

int
main(int argc, char** argv)
{
    int32_t fd;
    uint32_t nblocks;

    // input checking 
    if (argc != 3)
        Usage(argv[0]);

    // open input file
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    // use fstat to get file size
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1)
    {
        perror("fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // calculate nblocks
    off_t  file_size = fileStat.st_size; // total file size
    nblocks = file_size / BSIZE; // calculate number of blocks to read an entire file
    
    //printf(" no. of blocks = %u \n", nblocks);

    uint8_t* data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0); // map file to memory for read access
    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // calculate hash value of the input file
    int num_threads = atoi(argv[2]); // number of threads to create, convert to an integer
    size_t blocksPerThread = nblocks / num_threads; // split the block for hashing

    //double start = GetTime();
    //
    // root creation
    struct threadArg root_arg;
    root_arg.num_threads = num_threads;
    root_arg.thread_id = 0;
    root_arg.file_data = data;
    root_arg.size = blocksPerThread * BSIZE;
    
    printf("Num Threads = %d \n", num_threads);
    printf("Blocks Per Threads = %zu \n", blocksPerThread);
    
    pthread_t root; 
    Pthread_create(&root, NULL, tree, (void*)&root_arg);
    
    uint32_t* hash;
    Pthread_join(root, (void**)&hash);
    
    //double end = GetTime();

    printf("hash value = %u \n", *hash);
    //printf("time taken = %f \n", (end - start));
    
    pthread_exit(hash);
    munmap(data, file_size);
    close(fd);
    return EXIT_SUCCESS;
}

/*
 * This method recursively works to create a complete binary tree of threads.
 * It checks if a right child is needed and if so, it will create both left and right children.
 * Otherwise it checks for only if a left child needs to be created. Else, create the leaf thread.
 * */
void*
tree(void* arg) {
    struct threadArg* args = (struct threadArg*)arg; // access to the thread properties
    pthread_t left_child;
    pthread_t right_child;
    struct threadArg left_args = *args;
    struct threadArg right_args = *args;
    uint32_t* left_hash;
    uint32_t* right_hash;
    uint32_t* final_hash = malloc(sizeof(uint32_t));
    
    left_args.thread_id = 2 * left_args.thread_id + 1; // position for left thread
    right_args.thread_id = 2 * right_args.thread_id + 2; // position for right thread
    
    // Initialize left and right children
    if (right_args.thread_id < (*args).num_threads) { // Check if both children will need to be made
	Pthread_create(&left_child, NULL, tree, &left_args); // create right thread
	Pthread_create(&right_child, NULL, tree, &right_args); // create right thread
	uint32_t* current_hash = calculateHash(args); // calculate the hash value
        
	Pthread_join(left_child, (void**)&left_hash); // store the right thread hash value
	Pthread_join(right_child, (void**)&right_hash); // store the right thread hash value

        char hash_result[MAX_HASH_SIZE];
	//printf("tnum %d hash from left child %u\n", (*args).thread_id, *left_hash);
	//printf("tnum %d hash from right child %u\n", (*args).thread_id, *right_hash);
	
	sprintf(hash_result, "%u%u%u", *current_hash, *left_hash, *right_hash);
        *final_hash = jenkins_one_at_a_time_hash((uint8_t*)hash_result, strlen(hash_result));
        
	//printf("tnum %d concat string %s\n", (*args).thread_id, hash_result);
	//printf("tnum %d hash sent to parent %u\n", (*args).thread_id, *final_hash);
	
	free(current_hash);
	free(left_hash);
	free(right_hash);
    } else if (left_args.thread_id < (*args).num_threads) {
	Pthread_create(&left_child, NULL, tree, &left_args); // create the left thread only
	uint32_t* current_hash = calculateHash(args); // calculate the hash value
        
	Pthread_join(left_child, (void**)&left_hash); // store the left thread hash value
        
	char hash_result[MAX_HASH_SIZE];	
	//printf("tnum %d hash from left child %u\n", (*args).thread_id, *left_hash);
	
	sprintf(hash_result, "%u%u", *current_hash, *left_hash);
        *final_hash = jenkins_one_at_a_time_hash((uint8_t*)hash_result, strlen(hash_result));
	
	//printf("tnum %d concat string %s\n", (*args).thread_id, hash_result);
	//printf("tnum %d hash sent to parent %u\n", (*args).thread_id, *final_hash);
	
	free(current_hash);
	free(left_hash);
    } else {
        uint32_t* leaf_hash = calculateHash(args);
	*final_hash = *leaf_hash;
	
	//printf("tnum %d hash sent to parent %u\n", (*args).thread_id, *final_hash);
	
	free(leaf_hash);
    }

    return final_hash;
}

/*
 * This method calculates the hash of the given node. It reads the data mapped from memory and adjusts
 * the position accordingly with the offset to correct hash the correct parts in the file.
 * */
void* calculateHash(void* arg)
{
    struct threadArg* args = (struct threadArg*)arg; // access to the thread properties
    uint32_t* hash = malloc(sizeof(uint32_t)); // allocate memory on the heap and store the hash there

    if (hash == NULL) // check for memory alloc failure
    {
        perror("malloc");
        pthread_exit(NULL);
    }

    // get the starting address of the block for hashing
    uint64_t offset = (*args).size * (*args).thread_id; // file location offset
    uint8_t* start_addr = (*args).file_data + offset; // starting file addr + offset

    *hash = jenkins_one_at_a_time_hash(start_addr, (*args).size);
    //printf("tnum %i hash computed %u \n", (*args).thread_id, *hash);
    
    return hash;
}

uint32_t
jenkins_one_at_a_time_hash(const uint8_t* key, uint64_t length)
{
    uint64_t i = 0;
    uint32_t hash = 0;

    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

void
Usage(char* s)
{
    fprintf(stderr, "Usage: %s filename num_threads \n", s);
    exit(EXIT_FAILURE);
}
