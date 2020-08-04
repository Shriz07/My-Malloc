#ifndef MEMDEF_H
#define MEMDEF_H

#include "custom_unistd.h"
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef struct heap_t
{
    void *mem;

    struct block_t *head;

    size_t total_size;
    size_t free_size;
    size_t block_count;

    short init;
    uint32_t checksum;

} HEAP;

typedef struct block_t
{
    char left_fence[8];

    uint32_t checksum;

    struct block_t *prev;
    struct block_t *next;
    size_t data_size;

    short used;

    const char *filename;
    int line;

    char right_fence[8];
} BLOCK;


#define PAGE_SIZE 4096
#define BLOCK_SIZE sizeof(BLOCK)
#define HEAP_SIZE sizeof(HEAP)

enum pointer_type_t
{
    pointer_null,
    pointer_out_of_heap,
    pointer_control_block,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

int heap_setup(void);

void *heap_malloc(size_t count);
void *heap_calloc(size_t number, size_t size);
void heap_free(void *memblock);
void *heap_realloc(void *memblock, size_t size);

void *heap_malloc_debug(size_t count, int fileline, const char *filename);
void *heap_calloc_debug(size_t number, size_t size, int fileline,
                        const char *filename);
void *heap_realloc_debug(void *memblock, size_t size, int fileline,
                         const char *filename);

void *heap_malloc_aligned(size_t count);
void *heap_calloc_aligned(size_t number, size_t size);
void *heap_realloc_aligned(void *memblock, size_t size);

void *heap_malloc_aligned_debug(size_t count, int fileline, const char *filename);
void *heap_calloc_aligned_debug(size_t number, size_t size, int fileline, const char *filename);
void *heap_realloc_aligned_debug(void *memblock, size_t size, int fileline, const char *filename);

size_t heap_get_used_space(void);
size_t heap_get_largest_used_block_size(void);
uint64_t heap_get_used_blocks_count(void);
size_t heap_get_free_space(void);
size_t heap_get_largest_free_area(void);
uint64_t heap_get_free_gaps_count(void);

enum pointer_type_t get_pointer_type(const const void *pointer);
void *heap_get_data_block_start(const void *pointer);
size_t heap_get_block_size(const const void *memblock);
int heap_validate(void);
void heap_dump_debug_information(void);

void split_block(struct block_t *block, size_t size); //Function to split block
struct block_t *get_last_used_block(void); //Function to return last block of heap
int merge(void); //Function to delete metadata block between two free blocks
void set_fences_block(struct block_t *block); //Function to set fences in block
int check_checksum_block(struct block_t *block); //Function to check checksum in block
int check_checksum_heap(struct heap_t *heap); //Function to check checksum in heap
int check_fences_block(struct block_t *block); //Function to check fences in block
size_t heap_free_size(); //Function that calculates free size of heap
size_t blocks_data_size(); //Function that calculates free size in blocks
size_t caluclate_left_size_inside(void *ptr); //Function returns unallocated bytes in front of given pointer
size_t calculate_difference(void *ptr1, void *ptr2); //Funcion returns sum of bytes between ptr1 and ptr2
void extend_heap(int multi); //Function extends heap by multi * PAGE_SIZE
int aligned(void *ptr); //Funcion checks if given ptr is aligned
void heap_reset(); //Function reset heap to default state
void heap_destroy(); //Function destroys heap
uint32_t calculate_checksum(void *start, void *end); //Function calculate checksum with given start and end of block

#endif