#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "memdef.h"
#include "custom_unistd.h"
#include "unit_tests.h"

void error_msg(char *text, unsigned int line)
{
    printf("%s\n", text);
    printf("Line: %u\n", line);
}

//#### TEST 1 ####
//#### HEAP_SETUP TEST ####
//Test sprawdza poprawnosc inicjalizacji sterty funkcja heap_setup (Musi sie udac)

void test_1()
{
    printf("### TEST 1 ###\n");
    printf("### START ###\n");

    int check = heap_setup();
    if(check == -1)
    {
        error_msg("\nTEST 1\nFailed to initialize heap\n", __LINE__);
        assert(check == 0);
    }
    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 2 ####
//#### HEAP_MALLOC TEST ####
//Test sprawdza czy malloc zwraca null przy podaniu niepoprawnego rozmiaru

void test_2()
{
    printf("### TEST 2 ###\n");
    printf("### START ###\n");

    heap_setup();

    int *data1 = (int*)heap_malloc(0);
    if(data1 != NULL)
    {
        error_msg("\nTEST 2\nMalloc should return NULL but it returned a pointer\n", __LINE__);
        assert(data1 == NULL);
    }
    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 3 ####
//#### HEAP_MALLOC TEST ####
/*Test sprawdza poprawnosc alokowania danych przez funkcje malloc oraz dane zwracane
  przez funkcje statystyczne*/

void test_3()
{
    printf("### TEST 3 ###\n");
    printf("### START ###\n");

    heap_setup();

    int *data[5];
    size_t size = 0;
    size_t largest = 0;
    size_t blocks = 5;
    size_t free_space;
    size_t free_blocks = 1;
    size_t block_size[5];
    for(int i = 0; i < blocks; i++)
    {
        data[i] = (int*)heap_malloc(sizeof(int)*(1+(i * i)));
        block_size[i] = sizeof(int)*(1+(i * i));
        if(data[i] == NULL)
        {
            error_msg("\nTEST 3\nMalloc should return pointer but it returned NULL", __LINE__);
            assert(data[i] != NULL);
        }
        size += sizeof(int) * (1+(i * i));
        size += BLOCK_SIZE;
        if(i == 4)
            largest = sizeof(int) * (1 + (i*i));
    }
    free_space = PAGE_SIZE - size - BLOCK_SIZE;
    if(size != heap_get_used_space())
    {
        error_msg("\nTEST 3\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)size, (unsigned int)heap_get_used_space());
        assert(size == heap_get_used_space());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 3\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 3\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)blocks, (unsigned int)heap_get_used_blocks_count());
        assert(blocks == heap_get_used_blocks_count());
    }
    if(free_blocks != heap_get_free_gaps_count())
    {
        error_msg("\nTEST 3\nInvalid sum of free gaps\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_blocks, (unsigned int)heap_get_free_gaps_count());
        assert(blocks == heap_get_free_gaps_count());
    }
    if(free_space != heap_get_free_space())
    {
        error_msg("\nTEST 3\nInvalid sum of free space in heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_space, (unsigned int)heap_get_free_space());
        assert(blocks == heap_get_free_space());
    }
    for(int i = 0; i < blocks; i++)
    {
        if(block_size[i] != heap_get_block_size(data[i]))
        {
            error_msg("\nTEST 3\nInvalid size of block\n", __LINE__);
            printf("In pointer: %d | Should be: %u | but it is: %u\n", i, (unsigned int)block_size[i], (unsigned int)heap_get_block_size(data[i]));
            assert(block_size[i] == heap_get_block_size(data[i]));
        }
    }

    for(int i = 0; i < blocks; i++)
        heap_free(data[i]);
    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 4 ####
//#### HEAP_MALLOC TEST ####
//Test sprawdza poprawnosc alokowania duzych danych oraz funkcje statystyczne

void test_4()
{
    printf("### TEST 4 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();
    unsigned int used_blocks = 3;
    size_t largest = 8 * 1024 * 1024;
    size_t used = (8 * 1024 * 1024) * 3 + (3 * BLOCK_SIZE);
    
    void* p1 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p2 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p3 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p4 = heap_malloc(45 * 1024 * 1024); // 45MB
    if(p1 == NULL || p2 == NULL || p3 == NULL)
    {
        error_msg("\nTEST 4\nInvaild returned pointer\nShould return valid pointer but it returned NULL\n", __LINE__);
        assert(p1 != NULL);
        assert(p2 != NULL);
        assert(p3 != NULL);
    }
    if(p4 != NULL)
    {
        error_msg("\nTEST 4\nInvaild returned pointer\nShould return a NULL pointer\n", __LINE__);
        assert(p4 == NULL);
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 4\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 4\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used != heap_get_used_space())
    {
        error_msg("\nTEST 4\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used, (unsigned int)heap_get_used_space());
        assert(used == heap_get_used_space());
    }
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 4\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 4\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 4\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }
    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 5 ####
//#### HEAP_MALLOC TEST ####
//Test sprawdza poprawnosc alokowania danych oraz uzytecznosc wskaznikow

void test_5()
{
    printf("### TEST 5 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();
    size_t used_space = 200 + 4096 + (sizeof(int) * 2) + (3 * BLOCK_SIZE);
    size_t largest = 4096;
    unsigned int used_blocks = 3;

    char *data1 = (char*)heap_malloc(sizeof(char) * 200);
    int *data2 = (int*)heap_malloc(sizeof(int) * 2);
    char *data3 = (char*)heap_malloc(sizeof(char) * 4096);

    strcpy(data1, "This is test 5");
    *data2 = 10;
    *(data2 + 1) = 20;
    strcpy(data3, "This is test message");
    
    if(data1 == NULL || data2 == NULL || data3 == NULL)
    {
        error_msg("\nTEST 5\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(data1 != NULL);
        assert(data2 != NULL);
        assert(data3 != NULL);
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 5\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 5\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 5\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    if(strcmp(data1, "This is test 5") != 0)
    {
        error_msg("\nTEST 5\nInvalid data stored in pointer\n", __LINE__);
        printf("Should be: This is test 5 | but it is: %s\n", data1);
        assert(strcmp(data1, "This is test 5") == 0);
    }
    if(*data2 != 10 && *(data2 + 1) != 20)
    {
        error_msg("\nTEST 5\nInvalid data stored in pointer\n", __LINE__);
        printf("Should be: 10, 20 | but it is: %d %d\n", *data2, *(data2+1));
        assert(*data2 == 10);
        assert(*(data2+1) == 20);
    }
    if(strcmp(data3, "This is test message") != 0)
    {
        error_msg("\nTEST 5\nInvalid data stored in pointer\n", __LINE__);
        printf("Should be: This is test message | but it is: %s\n", data3);
        assert(strcmp(data3, "This is test message") == 0);
    }

    heap_free(data1);
    heap_free(data2);
    heap_free(data3);

    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 5\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 5\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 5\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 6 ####
//#### HEAP_CALLOC TEST ####
//Test sprawdza poprawnosc alokowania danych za pomoca heap_calloc

void test_6()
{
    printf("### TEST 6 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();
    size_t used_space = (sizeof(int) * 500) + (sizeof(int) * 250) + (sizeof(double) * 1000) + (3 * BLOCK_SIZE);
    size_t largest = sizeof(double) * 1000;
    unsigned int used_blocks = 3;
    int should_be = 0;

    int *data1 = (int*)heap_calloc(-1, 23);
    char *data2 = (char*)heap_calloc(0, 5);
    int *data3 = (int*)heap_calloc(sizeof(int), 500);
    int *data4 = (int*)heap_calloc(sizeof(int), 250);
    double *data5 = (double*)heap_calloc(sizeof(double), 1000);

    if(data1 != NULL || data2 != NULL)
    {
        error_msg("\nTEST 6\nInvalid returned pointer\nShould be NULL\n", __LINE__);
        assert(data1 == NULL);
        assert(data2 == NULL);
    }
    if(data3 == NULL || data4 == NULL || data5 == NULL)
    {
        error_msg("\nTEST 6\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(data3 != NULL);
        assert(data4 != NULL);
        assert(data5 != NULL);
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 6\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 6\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 6\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    for(int i = 0; i < 500; i++)
    {
        if(*(data3 + i) != should_be)
        {
            error_msg("\nTEST 6\nInvalid data stored in pointer data3\n", __LINE__);
            printf("Should be: 0 | but it is: %d\n", *(data3 + i));
            assert(*(data3 + i) == should_be);
        }
    }
    for(int i = 0; i < 250; i++)
    {
        if(*(data4 + i) != should_be)
        {
            error_msg("\nTEST 6\nInvalid data stored in pointer data4\n", __LINE__);
            printf("Should be: 0 | but it is: %d\n", *(data4 + i));
            assert(*(data4 + i) == should_be);
        }
    }
    for(int i = 0; i < 1000; i++)
    {
        if(*(data5 + i) != should_be)
        {
            error_msg("\nTEST 6\nInvalid data stored in pointer data5\n", __LINE__);
            printf("Should be: 0 | but it is: %f\n", *(data5 + i));
            assert(*(data5 + i) == should_be);
        }
    }

    heap_free(data3);
    heap_free(data4);
    heap_free(data5);

    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 6\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 6\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 6\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 7 ####
//#### HEAP_REALLOC TEST ####
//Test sprawdza poprawnosc dzialania heap_realloc

void test_7()
{
    printf("### TEST 7 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();
    size_t used_space = (sizeof(double) * 300) + (sizeof(int) * 500) + (sizeof(char) * 4000) + (sizeof(int) * 5) + (4 * BLOCK_SIZE);
    size_t largest = 4000;
    unsigned int used_blocks = 4;

    //Allocating memmory
    double *data1 = (double*)heap_realloc(NULL, sizeof(double) * 300);
    int *data2 = (int*)heap_realloc(NULL, sizeof(int) * 500);
    char *data3 = (char*)heap_realloc(NULL, sizeof(char) * 4000);
    int *data4 = (int*)heap_realloc(NULL, sizeof(int) * 5);

    //Checking pointers
    if(data1 == NULL || data2 == NULL || data3 == NULL || data4 == NULL)
    {
        error_msg("\nTEST 7\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(data1 != NULL);
        assert(data2 != NULL);
        assert(data3 != NULL);
        assert(data4 != NULL);
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 7\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 7\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 7\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }

    //Reallocing first data
    double *new_data1 = (double*)heap_realloc(data1, sizeof(double) * 600);
    used_space += ((sizeof(double) * 600) + BLOCK_SIZE);
    largest = sizeof(double) * 600;

    //Check if reallocation was successful
    if(new_data1 == NULL)
    {
        error_msg("\nTEST 7\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(new_data1 != NULL);
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 7\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 7\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }

    //Reallocating third data
    strcpy(data3, "This is test message");
    char *new_data3 = (char*)heap_realloc(data3, sizeof(char) * 4100);
    used_space += (sizeof(char) * 4100 + BLOCK_SIZE);

    //Check if reallocation was successful
    if(new_data3 == NULL)
    {
        error_msg("\nTEST 7\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(new_data3 != NULL);
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 7\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    //Check if data after reallocation was copied
    if(strcmp(new_data3, "This is test message") != 0)
    {
        error_msg("\nTEST 7\nInvalid data stored in pointer\n", __LINE__);
        printf("Should be: This is test message | but it is: %s\n", new_data3);
        assert(strcmp(new_data3, "This is test message") == 0);
    }

    //Freeing data using realloc
    heap_realloc(new_data1, 0);
    heap_realloc(data2, 0);
    heap_realloc(new_data3, 0);
    heap_realloc(data4, 0);

    //Check if realloc freed memory correctly
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 7\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 7\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 7\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 8 ####
//#### HEAP_MALLOC_ALIGNED TEST ####
//Test sprawdza czy malloc_aligned poprawnie alokuje pamiec i zwraca poprawne wskazniki

void test_8()
{
    printf("### TEST 8 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();

    extend_heap(2);


    unsigned int used_blocks = 2;
    size_t largest = sizeof(int) * 200;
    size_t gap1 = PAGE_SIZE - (2 * BLOCK_SIZE);
    size_t gap2 = PAGE_SIZE - (sizeof(char) * 500) - BLOCK_SIZE;
    size_t used = BLOCK_SIZE + gap1 + BLOCK_SIZE + (sizeof(char) * 500)+ gap2 + BLOCK_SIZE + (sizeof(int) * 200);

    //Allocating memory
    int *data1 = (int*)heap_malloc_aligned(0);
    char *data2 = (char*)heap_malloc_aligned(sizeof(char) * 500);
    int *data3 = (int*)heap_malloc_aligned(sizeof(int) * 200);

    //Check if returned pointers are correct and aligned
    if(data1 != NULL)
    {
        error_msg("\nTEST 8\nInvaild returned pointer\nShould return a NULL pointer\n", __LINE__);
        assert(data1 == NULL);
    }
    if(aligned((void*)data2) == 0)
    {
        error_msg("\nTEST 8\nInvaild returned pointer\nPointer data2 is not aligned\n", __LINE__);
        assert((aligned((void*)data2) != 0));
    }
    if(aligned((void*)data3) == 0)
    {
        error_msg("\nTEST 8\nInvaild returned pointer\nPointer data3 is not aligned\n", __LINE__);
        assert((aligned((void*)data3) != 0));
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 8\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 8\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used != heap_get_used_space())
    {
        error_msg("\nTEST 8\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used, (unsigned int)heap_get_used_space());
        assert(used == heap_get_used_space());
    }  

    heap_free(data2);
    heap_free(data3);

    //Check if free was successful
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 8\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 8\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 8\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 9 ####
//#### HEAP_CALLOC_ALIGNED TEST ####
//Test sprawdza czy calloc_aligned poprawnie alokuje pamiec i zwraca poprawne wskazniki oraz przypisuje poprawne wartosci

void test_9()
{
    printf("### TEST 9 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();

    extend_heap(2);


    unsigned int used_blocks = 2;
    size_t largest = sizeof(int) * 200;
    size_t gap1 = PAGE_SIZE - (2 * BLOCK_SIZE);
    size_t gap2 = PAGE_SIZE - (sizeof(char) * 500) - BLOCK_SIZE;
    size_t used = BLOCK_SIZE + gap1 + BLOCK_SIZE + (sizeof(char) * 500)+ gap2 + BLOCK_SIZE + (sizeof(int) * 200);

    //Allocating memory
    int *data1 = (int*)heap_calloc_aligned(0, 5);
    char *data2 = (char*)heap_calloc_aligned(sizeof(char), 500);
    int *data3 = (int*)heap_calloc_aligned(sizeof(int), 200);

    //Check if returned pointers are correct and aligned
    if(data1 != NULL)
    {
        error_msg("\nTEST 9\nInvaild returned pointer\nShould return a NULL pointer\n", __LINE__);
        assert(data1 == NULL);
    }
    if(aligned((void*)data2) == 0)
    {
        error_msg("\nTEST 9\nInvaild returned pointer\nPointer data2 is not aligned\n", __LINE__);
        assert((aligned((void*)data2) != 0));
    }
    if(aligned((void*)data3) == 0)
    {
        error_msg("\nTEST 9\nInvaild returned pointer\nPointer data3 is not aligned\n", __LINE__);
        assert((aligned((void*)data3) != 0));
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 9\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 9\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used != heap_get_used_space())
    {
        error_msg("\nTEST 9\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used, (unsigned int)heap_get_used_space());
        assert(used == heap_get_used_space());
    }

    //Adding some data
    strcpy(data2, "This is test message");
    for(int i = 0; i < 5; i++)
        *(data3 + i) = i;

    //Checking if data is stored correctly
    if(strcmp(data2, "This is test message") != 0)
    {
        error_msg("\nTEST 9\nInvalid data stored in pointer data2\n", __LINE__);
        printf("Should be: This is test message | but it is: %s\n", data2);
        assert(strcmp(data2, "This is test message") == 0);
    }
    for(int i = 0; i < 5; i++)
    {
        if(*(data3 + i) != i)
        {
            error_msg("\nTEST 9\nInvalid data stored in pointer data3\n", __LINE__);
            printf("In index %d should be: %d | but it is: %d\n", i, i, *(data3 + i));
            assert(*(data3 + i) == i);
        }
    }

    heap_free(data2);
    heap_free(data3);

    //Check if free was successful
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 9\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 9\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 9\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 10 ####
//#### HEAP_REALLOC_ALIGNED TEST ####
//Test sprawdza czy realloc_aligned poprawnie alokuje pamiec i zwraca poprawne wskazniki

void test_10()
{
    printf("### TEST 10 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();

    extend_heap(2);


    unsigned int used_blocks = 2;
    size_t largest = sizeof(int) * 200;
    size_t gap1 = PAGE_SIZE - (2 * BLOCK_SIZE);
    size_t gap2 = PAGE_SIZE - (sizeof(int) * 200) - BLOCK_SIZE;
    size_t used = BLOCK_SIZE + gap1 + BLOCK_SIZE + (sizeof(int) * 200)+ gap2 + BLOCK_SIZE + (sizeof(char) * 500);

    //Allocating memory
    int *data1 = (int*)heap_realloc_aligned(NULL, sizeof(int) * 200);
    char *data2 = (char*)heap_realloc_aligned(NULL, sizeof(char) * 500);
    //Check if returned pointers are correct and aligned
    if(aligned((void*)data1) == 0)
    {
        error_msg("\nTEST 10\nInvaild returned pointer\nPointer data1 is not aligned\n", __LINE__);
        assert((aligned((void*)data1) != 0));
    }
    if(aligned((void*)data2) == 0)
    {
        error_msg("\nTEST 10\nInvaild returned pointer\nPointer data2 is not aligned\n", __LINE__);
        assert((aligned((void*)data2) != 0));
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 10\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 10\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used != heap_get_used_space())
    {
        error_msg("\nTEST 10\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used, (unsigned int)heap_get_used_space());
        assert(used == heap_get_used_space());
    }
    //Adding some data
    strcpy(data2, "This is test message");
    for(int i = 0; i < 5; i++)
        *(data1 + i) = i;


    char *new_data2  = (char*)heap_realloc_aligned(data2, sizeof(char) * 5000);
    if(new_data2 != NULL) //Not enough memory in heap
    {
        error_msg("\nTEST 10\nInvaild returned pointer\nShould return a NULL pointer\n", __LINE__);
        assert(new_data2 == NULL);
    }
    extend_heap(1);
    int *new_data1 = (int*)heap_realloc_aligned(data1, sizeof(int) * 400);
    new_data2 = (char*)heap_realloc_aligned(data2, sizeof(char) * 5000);
    largest = sizeof(char) * 5000;
    gap2 = PAGE_SIZE - (sizeof(int) * 400) - BLOCK_SIZE;
    used = BLOCK_SIZE + gap1 + BLOCK_SIZE + (sizeof(int) * 400) + gap2 + BLOCK_SIZE + (sizeof(char) * 5000);

    //Check if returned pointers are correct and aligned
    if(aligned((void*)new_data1) == 0)
    {
        error_msg("\nTEST 10\nInvaild returned pointer\nPointer new_data1 is not aligned\n", __LINE__);
        assert((aligned((void*)new_data1) != 0));
    }
    if(aligned((void*)new_data2) == 0)
    {
        error_msg("\nTEST 10\nInvaild returned pointer\nPointer new_data2 is not aligned\n", __LINE__);
        assert((aligned((void*)new_data2) != 0));
    }
    if(used != heap_get_used_space())
    {
        error_msg("\nTEST 10\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used, (unsigned int)heap_get_used_space());
        assert(used == heap_get_used_space());
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 10\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 10\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    //Checking if data is stored correctly
    if(strcmp(new_data2, "This is test message") != 0)
    {
        error_msg("\nTEST 10\nInvalid data stored in pointer new_data2\n", __LINE__);
        printf("Should be: This is test message | but it is: %s\n", new_data2);
        assert(strcmp(new_data2, "This is test message") == 0);
    }
    for(int i = 0; i < 5; i++)
    {
        if(*(new_data1 + i) != i)
        {
            error_msg("\nTEST 10\nInvalid data stored in pointer new_data1\n", __LINE__);
            printf("In index %d should be: %d | but it is: %d\n", i, i, *(new_data1 + i));
            assert(*(new_data1 + i) == i);
        }
    }

    heap_realloc_aligned(new_data1, 0);
    heap_realloc_aligned(new_data2, 0);

    //Check if free was successful
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 10\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 10\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 10\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}

//#### TEST 11 ####
//#### HEAP_FREE TEST ####
//Test sprawdza czy heap_free poprawnie zwalnia pamiec

void test_11()
{
    printf("### TEST 11 ###\n");
    printf("### START ###\n");

    heap_setup();

    size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();
    size_t used_space = (sizeof(int) * 50) + (sizeof(int) * 3000) + (sizeof(int) * 2000) + (sizeof(char) * 4000) + (4 * BLOCK_SIZE);
    size_t largest = sizeof(int) * 3000;
    unsigned int used_blocks = 4;

    int *data1 = (int*)heap_malloc(sizeof(int) * 50);
    int *data2 = (int*)heap_malloc(sizeof(int) * 3000);
    int *data3 = (int*)heap_malloc(sizeof(int) * 2000);
    char *data4 = (char*)heap_malloc(sizeof(char) * 4000);

    //Checking pointers
    if(data1 == NULL || data2 == NULL || data3 == NULL || data4 == NULL)
    {
        error_msg("\nTEST 11\nInvalid returned pointer\nPointer should be valid but it is NULL\n", __LINE__);
        assert(data1 != NULL);
        assert(data2 != NULL);
        assert(data3 != NULL);
        assert(data4 != NULL);
    }
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 11\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 11\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 11\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    //Free pointer data2 and NULL 
    heap_free(data2);
    heap_free(NULL);
    largest = sizeof(int) * 2000;
    used_blocks = 3;

    //Check if free was correct
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 11\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 11\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 11\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    //Free pointer data4
    heap_free(data4);
    used_space -= ((sizeof(char) * 4000) + BLOCK_SIZE);
    used_blocks = 2;
    //Check if free was correct
    if(used_blocks != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 11\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_blocks, (unsigned int)heap_get_used_blocks_count());
        assert(used_blocks == heap_get_used_blocks_count());
    }
    if(largest != heap_get_largest_used_block_size())
    {
        error_msg("\nTEST 11\nInvalid largest used block\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)largest, (unsigned int)heap_get_largest_used_block_size());
        assert(largest == heap_get_largest_used_block_size());
    }
    if(used_space != heap_get_used_space())
    {
        error_msg("\nTEST 11\nInvalid used space of heap\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_space, (unsigned int)heap_get_used_space());
        assert(used_space == heap_get_used_space());
    }
    //Free pointers data1 and data3
    heap_free(data1);
    heap_free(data3);

    //Check if free was successful
    if(free_bytes != heap_get_free_space())
    {
        error_msg("\nTEST 11\nInvalid sum of free bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)free_bytes, (unsigned int)heap_get_free_space());
        assert(free_bytes == heap_get_used_space());
    }
    if(used_bytes != heap_get_used_space())
    {
        error_msg("\nTEST 11\nInvalid sum of used bytes\n", __LINE__);
        printf("Should be: %u | but it is: %u\n", (unsigned int)used_bytes, (unsigned int)heap_get_used_space());
        assert(used_bytes == heap_get_used_space());
    }
    if(0 != heap_get_used_blocks_count())
    {
        error_msg("\nTEST 11\nInvalid sum of used blocks\n", __LINE__);
        printf("Should be: 0 | but it is: %u\n", (unsigned int)heap_get_used_blocks_count());
        assert(0 == heap_get_used_blocks_count());
    }

    heap_destroy();
    printf("### END ###\n");
}
