#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include "custom_unistd.h"
#include "memdef.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
HEAP heap;

int heap_setup(void)
{
    void *mem = custom_sbrk(PAGE_SIZE);
    
    if(mem == (void*)-1)
        return -1;

    heap.mem = mem;

    BLOCK *head = (BLOCK *)mem;

    head->data_size = PAGE_SIZE - BLOCK_SIZE;
    head->next = NULL;
    head->prev = NULL;
    head->used = 0;
    set_fences_block(head);

    head->checksum = 0;
    head->checksum = calculate_checksum(head, (head+1));

    heap.head = head;

    heap.total_size = PAGE_SIZE;
    heap.free_size = PAGE_SIZE - BLOCK_SIZE;
    heap.block_count = 1;
    heap.init = 1;

    heap.checksum = 0;
    heap.checksum = calculate_checksum(&heap, ((&heap) + 1));

    assert(heap_validate() == 0);

    return 0;
}

void *heap_calloc_debug(size_t number, size_t size, int fileline, const char *filename)
{
    if(heap.init == 0 || number <= 0 || size <= 0)
        return NULL;
    size_t check = number * size;
    if(check/number != size)
        return NULL;
    
    assert(heap_validate() == 0);
    void *mem = heap_malloc_debug(check, fileline, filename);
    if(mem)
        memset(mem, 0, check);
    return mem;
}

void *heap_calloc(size_t number, size_t size)
{
    return heap_calloc_debug(number, size, 0, NULL);
}

void *heap_realloc_debug(void *memblock, size_t size, int fileline, const char *filename)
{
    if(heap.init == 0)
        return NULL;
    if(memblock == NULL)
        return heap_malloc_debug(size, fileline, filename);
    if(!size)
    {
        heap_free(memblock);
        return NULL;
    }
    assert(heap_validate() == 0);
    assert(get_pointer_type(memblock) == pointer_valid);
    void *result;
    pthread_mutex_lock(&mutex);
    BLOCK *block = (BLOCK*)memblock - 1;
    intptr_t memory_needed = size - block->data_size;
    block->filename = filename;
    block->line = fileline;

    if(memory_needed < 0)
    {
        BLOCK *new_block = (void*)((void*)block + size + BLOCK_SIZE);
        new_block->used = 0;
        new_block->prev = block;
        new_block->data_size = (-memory_needed);
        new_block->next = block->next;
        set_fences_block(new_block);
        new_block->checksum = 0;
        new_block->checksum = calculate_checksum(new_block, (new_block + 1));

        block->filename = filename;
        block->line = fileline;
        block->next = new_block;
        block->data_size = size;
        set_fences_block(block);
        block->checksum = 0;
        block->checksum = calculate_checksum(block, (block+1));

        heap.free_size = heap.free_size - memory_needed;
        heap.checksum = 0;
        heap.checksum = calculate_checksum(&heap, ((&heap)+1));
        merge();
        pthread_mutex_unlock(&mutex);
        result = (void*)(++block);
        return result; 
    }
    else if(memory_needed > 0)
    {
        if(block->next->used == 0 && block->next->data_size + BLOCK_SIZE >= memory_needed)
        {
            BLOCK *new_block = (void*)((void*)block + size + BLOCK_SIZE);
            new_block->used = 0;
            new_block->prev = block;
            new_block->next = block->next->next;
            if(block->next->next != NULL)
                block->next->next->prev = new_block;
            new_block->data_size = block->next->data_size - memory_needed;
            set_fences_block(new_block);
            new_block->checksum = 0;
            new_block->checksum = calculate_checksum(new_block, (new_block + 1));

            block->filename = filename;
            block->line = fileline;
            block->next = new_block;
            block->data_size = size;
            set_fences_block(block);
            block->checksum = 0;
            block->checksum = calculate_checksum(block, (block+1));

            heap.free_size = heap.free_size + (-memory_needed);
            heap.checksum = 0;
            heap.checksum = calculate_checksum(&heap, ((&heap) + 1));

            pthread_mutex_unlock(&mutex);
            result = (void*)(++block);
            return block;
        }
        else //malloc and copy
        {
            pthread_mutex_unlock(&mutex);
            void *new_mem = heap_malloc_debug(size, fileline, filename);
            if(new_mem == NULL)
                return NULL;
            memcpy(new_mem, memblock, block->data_size);
            heap_free(memblock);
            return new_mem;
        }
    }
    else
    {
        pthread_mutex_unlock(&mutex);
        return memblock;
    }
}

void *heap_realloc(void *memblock, size_t size)
{
    return heap_realloc_debug(memblock, size, 0, NULL);
}

void *heap_malloc_debug(size_t count, int fileline, const char *filename)
{
    if(count == 0 || heap.init == 0)
        return NULL;

    assert(heap_validate() == 0);
    pthread_mutex_lock(&mutex);


    void *result;

    BLOCK *curr;
    curr = heap.head;

    int found = 0;

    while(!found) //Finding block
    {
        if(curr->data_size == count && curr->next != NULL && curr->used == 0)
            break;
        if(curr->next != NULL && curr->data_size >= (count + BLOCK_SIZE))
            found = 1;
        if(curr->used)
            found = 0;
        if(curr->next == NULL)
            break;
        if(found)
            break;
        curr = curr->next;
    }

    if(curr->data_size == count) //Block exacly fits required size
    {
        curr->used = 1;
        curr->filename = filename;
        curr->line = fileline;
        result = (void *)(++curr); //++ to skip metadata block
        heap.free_size = heap.free_size - count - BLOCK_SIZE;
        heap.checksum = 0;
        heap.checksum = calculate_checksum(&heap, ((&heap) + 1));
        pthread_mutex_unlock(&mutex);
        return result;
    }
    else if(curr->data_size > (count + BLOCK_SIZE)) //Split blocks
    {
        split_block(curr, count);
        curr->used = 1;
        curr->filename = filename;
        curr->line = fileline;
        result = (void *)(++curr);
        heap.free_size = heap.free_size - count - BLOCK_SIZE;
        heap.checksum = 0;
        heap.checksum = calculate_checksum(&heap, ((&heap) + 1));
        pthread_mutex_unlock(&mutex);
        return result;
    }
    else //not enough memory
    {
        pthread_mutex_unlock(&mutex);
        size_t memory_needed = count + BLOCK_SIZE;
        BLOCK *end = get_last_used_block();
        memory_needed -= end->data_size;
        int multi = 1;
        multi = multi + (memory_needed / PAGE_SIZE);
        void *check = custom_sbrk((PAGE_SIZE * multi));
        if(check == ((void*)-1))
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        heap.total_size = heap.total_size + (PAGE_SIZE * multi);
        heap.free_size = heap.free_size + (PAGE_SIZE * multi);
        heap.checksum = 0;
        heap.checksum = calculate_checksum(&heap, ((&heap) + 1));


        end->filename = NULL;
        end->line = 0;
        end->data_size = end->data_size + (PAGE_SIZE * multi);
        end->checksum = 0;
        set_fences_block(end);
        end->checksum = calculate_checksum(end, (end+1));
        return heap_malloc_debug(count, fileline, filename);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *heap_realloc_aligned_debug(void *memblock, size_t size, int fileline, const char *filename)
{
    if(heap.init == 0)
        return NULL;
    if(memblock == NULL)
        return heap_malloc_aligned_debug(size, fileline, filename);
    assert(heap_validate() == 0);
    pthread_mutex_lock(&mutex);
    if(!size)
    {
        pthread_mutex_unlock(&mutex);
        heap_free(memblock);
        return NULL;
    }
    BLOCK *block = (BLOCK*)memblock - 1;
    size_t need = size - block->data_size;
    block->filename = filename;
    block->line = fileline;
    if(block->next->used == 0 && (block->next->data_size - BLOCK_SIZE) > need)
    {
        BLOCK *new_block = (void*)((void*)block + size + BLOCK_SIZE);
        new_block->used = 0;
        new_block->prev = block;
        new_block->next = block->next->next;
        if(block->next->next != NULL)
        {
            BLOCK *tmp = block->next->next;
            tmp->prev = new_block;
            tmp->checksum = 0;
            tmp->checksum = calculate_checksum(tmp, (tmp + 1));
        }
        new_block->data_size = block->next->data_size - need;
        set_fences_block(new_block);
        new_block->checksum = 0;
        new_block->checksum = calculate_checksum(new_block, (new_block + 1));

        block->data_size = size;
        block->next = new_block;
        set_fences_block(block);
        block->checksum = 0;
        block->checksum = calculate_checksum(block, (block+1));

        heap.free_size = heap.free_size - need;
        heap.checksum = 0;
        heap.checksum = calculate_checksum(&heap, ((&heap) +1));
        pthread_mutex_unlock(&mutex);
        void *result = (void*)(++block);
        return result;
    }
    pthread_mutex_unlock(&mutex);
    void *mem = heap_malloc_aligned_debug(size, fileline, filename);
    if(mem == NULL)
    {
        return NULL;
    }
    memcpy(mem, memblock, block->data_size);
    heap_free(memblock);
    return mem;
}

void *heap_realloc_aligned(void *memblock, size_t size)
{
    return heap_realloc_aligned_debug(memblock, size, 0, NULL);
}

void *heap_calloc_aligned_debug(size_t number, size_t size, int fileline, const char *filename)
{
    if(heap.init == 0 || number == 0 || size == 0)
        return NULL;
    size_t check = number * size;
    if(check/number != size)
        return NULL;

    assert(heap_validate() == 0);
    void *mem = heap_malloc_aligned_debug(check, fileline, filename);
    if(mem)
        memset(mem, 0, check);
    return mem;
}
void *heap_calloc_aligned(size_t number, size_t size)
{
    return heap_calloc_aligned_debug(number, size, 0, NULL);
}

void *heap_malloc_aligned_debug(size_t count, int fileline, const char *filename)
{
    if(count == 0 || heap.init == 0)
        return NULL;

    assert(heap_validate() == 0);
    pthread_mutex_lock(&mutex);

    void *result = NULL;
    void *ptr = (void*)heap.head;

    while(get_pointer_type(ptr) != pointer_out_of_heap)
    {
        if(get_pointer_type(ptr) == pointer_unallocated)
        {
            int space_before = 0;
            if(get_pointer_type((ptr - 1)) == pointer_unallocated && get_pointer_type((ptr - BLOCK_SIZE)) == pointer_unallocated)
                space_before = 1;

            int space_after = 1;
            for(int i = 0; i < count + BLOCK_SIZE; i += BLOCK_SIZE)
            {
                if(get_pointer_type((ptr + i)) != pointer_unallocated)
                {
                    space_after = 0;
                    break;
                }
            }
            if(space_before && space_after) //Adding block behind, current block and end block
            {
                BLOCK *block = (BLOCK *)(ptr - BLOCK_SIZE);
                block->filename = filename;
                block->line = fileline;
                block->used = 1;
                block->data_size = count;
                set_fences_block(block);

                int end_block = 0;
                void *tmp = ptr + count;
                while(get_pointer_type(tmp++) == pointer_unallocated);
                if(get_pointer_type(--tmp) != pointer_out_of_heap)
                    end_block = 1;

                BLOCK *other_end = NULL;
                if(end_block)
                {
                    other_end = (BLOCK *)(tmp);
                }
                BLOCK *end = (BLOCK*)(ptr + count);
                end->used = 0;
                end->filename = NULL;
                end->line = 0;
                set_fences_block(end);
                if(end_block)
                {
                    end->next = other_end;
                    other_end->prev = end;
                    other_end->checksum = 0;
                    other_end->checksum = calculate_checksum(other_end, (other_end + 1));
                }
                else
                    end->next = NULL;
                end->prev = block;
                end->checksum = 0;
                end->data_size = caluclate_left_size_inside((end + 1));
                end->checksum = calculate_checksum(end, (end + 1));

                void *behind = ptr;
                behind -= BLOCK_SIZE;
                while(get_pointer_type(behind -1) == pointer_unallocated)
                    behind--;
                BLOCK *prev = (BLOCK *)behind;
                prev -= 1;
                prev->next = block;
                prev->data_size = calculate_difference((void*)(prev + 1), (void *)block);
                prev->checksum = 0;
                prev->checksum = calculate_checksum(prev, (prev+1));
                block->next = end;
                block->prev = prev;
                block->checksum = 0;
                block->checksum = calculate_checksum(block, (block+1));

                heap.free_size = heap_free_size();
                heap.block_count += 2;
                heap.checksum = 0;
                heap.checksum = calculate_checksum(&heap, ((&heap) + 1));

                pthread_mutex_unlock(&mutex);
                result = (void*)(++block);
                return result;
            }
        }
        ptr += PAGE_SIZE;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

size_t caluclate_left_size_inside(void *ptr)
{
    size_t size = 0;
    while(get_pointer_type(ptr) == pointer_unallocated)
    {
        size++;
        ptr++;
    }
    return size;
}

size_t calculate_difference(void *ptr1, void *ptr2)
{
    size_t size = 0;
    while(ptr1 != ptr2)
    {
        size++;
        ptr1++;
    }
    return size;
}

void *heap_malloc_aligned(size_t count)
{
    return heap_malloc_aligned_debug(count, 0, NULL);
}

void extend_heap(int multi)
{
    if(heap.init == 0 || multi <= 0)
        return;
    
    custom_sbrk(PAGE_SIZE * multi);

    heap.total_size = heap.total_size + (PAGE_SIZE * multi);
    heap.free_size = heap.free_size + (PAGE_SIZE * multi);
    heap.checksum = 0;
    heap.checksum = calculate_checksum(&heap, ((&heap) + 1));

    BLOCK *end = get_last_used_block();

    end->data_size = end->data_size + (PAGE_SIZE * multi);
    end->checksum = 0;
    set_fences_block(end);
    end->checksum = calculate_checksum(end, (end+1));
}

void set_fences_block(struct block_t *block)
{
    for(int i = 0; i < 8; i++)
    {
        block->left_fence[i] = i;
        block->right_fence[i] = i;
    }
}

size_t heap_free_size()
{
    BLOCK *curr = heap.head;
    size_t free_size = 0;
    while(curr != NULL)
    {
        if(curr->used == 0)
            free_size += curr->data_size;
        curr = curr->next;
    }
    return free_size;
}

size_t blocks_data_size()
{
    BLOCK *curr = heap.head;
    size_t free_size = 0;
    while(curr != NULL)
    {
        if(curr->used == 0)
            free_size += curr->data_size;
        curr = curr->next;
    }
    return free_size;
}

void *heap_malloc(size_t count)
{
    return heap_malloc_debug(count, 0, NULL);
}

void heap_free(void *memblock)
{
    if(get_pointer_type(memblock) == pointer_out_of_heap || get_pointer_type(memblock) == pointer_null)
        return ;

    pthread_mutex_lock(&mutex);
    BLOCK *curr = memblock;
    --curr;
    curr->used = 0;
    heap.free_size += curr->data_size;
    while(merge());

    curr->checksum = 0;
    curr->checksum = calculate_checksum(curr, (curr+1));

    heap.checksum = 0;
    heap.checksum = calculate_checksum(&heap, ((&heap) + 1));

    heap_reset();
    pthread_mutex_unlock(&mutex);
}

void split_block(struct block_t *block, size_t size) //Using first fit
{
    BLOCK *new_block = (void *)((void*)block + size + BLOCK_SIZE); //Nowy blok koncowy
    new_block->data_size = block->data_size - size - BLOCK_SIZE; //Nowy rozmiar pomniejszony o size i rozmiar struktury na metadane
    new_block->used = 0;
    new_block->prev = block;
    new_block->next = NULL;
    if(block->next)
    {
        new_block->next = block->next;
        block->next->prev = new_block;
        block->next->checksum = 0;
        block->next->checksum = calculate_checksum(block->next, (block->next + 1));
    }
    set_fences_block(new_block);

    new_block->checksum = 0;
    new_block->checksum = calculate_checksum(new_block, (new_block+1));

    block->next = new_block;
    block->data_size = size;
    block->used = 1;
    heap.block_count += 1;
    set_fences_block(block);
    block->checksum = 0;
    block->checksum = calculate_checksum(block, (block+1));
}

int aligned(void *ptr)
{
    if(((intptr_t)ptr & (intptr_t)(PAGE_SIZE - 1)) == 0)
        return 1;
    return 0;
}

int merge(void) //Function to remove metadata block between two free blocks
{
    if(heap.init == 0 || heap.free_size != blocks_data_size())
        return 0;
    BLOCK *curr;
    curr = heap.head;

    int merged = 0;

    while(curr != NULL && curr->next != NULL)
    {
        if(curr->used == 0 && curr->next->used == 0)
        {
            heap.free_size = heap.free_size + BLOCK_SIZE; //Expand free memory size
            curr->data_size = curr->data_size + curr->next->data_size + BLOCK_SIZE;
            curr->next = curr->next->next;
            set_fences_block(curr);
            curr->checksum = 0;
            curr->checksum = calculate_checksum(curr, curr+1);
            merged++;
        }
        curr = curr->next;
    }
    heap.block_count -= merged;
    return merged;
}

int heap_validate(void)
{
    if(!heap.init)
        return -1;

    if(check_checksum_heap(&heap))
        return -1;

    BLOCK *curr = heap.head;
    BLOCK *prev = NULL;
    size_t free_size = 0;

    for(int i = 0; i < heap.block_count; i++)
    {
        if(curr == NULL)
        {
            //Invalid sum of blocks in heap
            return -2;
        }
        if(check_fences_block(curr))
        {
            //Invalid fences in block
            return -3;
        }
        if(check_checksum_block(curr))
        {
            //Invalid block checksum
            return -4;
        }
        if(curr->prev != prev && !prev)
        {
            //Invalid previous pointer in block
            return -5;
        }
        if(heap.free_size != heap_free_size())
        {
            //Invalid free size in heap
            return -6;
        }
        if(curr->used == 0)
            free_size += curr->data_size;
        prev = curr;
        curr = curr -> next;
    }
    if(free_size != heap.free_size)
        return -6;
    return 0;
}

int check_checksum_block(struct block_t *block)
{
    uint32_t check = block->checksum;
    block->checksum = 0;
    block->checksum = calculate_checksum(block, (block + 1));
    if(check != block->checksum)
    {
        printf("Current: %u | Calculated: %u\n", (unsigned int)check, block->checksum);
        return 1;
    }
    return 0;
}

int check_checksum_heap(struct heap_t *heap)
{
    uint32_t check = heap->checksum;
    heap->checksum = 0;
    heap->checksum = calculate_checksum(heap, (heap + 1));
    if(check != heap->checksum)
        return 1;
    return 0;
}

int check_fences_block(struct block_t *block)
{
    for(int i = 0; i < 8; i++)
    {
        if(block->left_fence[i] != i || block->right_fence[i] != i)
            return 1;
    }
    return 0;
}

uint32_t calculate_checksum(void *start, void *end)
{
    uint32_t sum = 0;
    for(uint8_t *curr = (uint8_t*)start; curr < (uint8_t*)end; curr++)
        sum += *curr;
    return sum;
}

uint32_t calculate_checksum_block(struct block_t *block, size_t size)
{
    uint32_t sum = 0;

    for(int i = 0; i < size; i++)
    {
        sum += *((((char *)block) + i));
    }

    return sum;
}

uint32_t calculate_checksum_heap(struct heap_t *heap, size_t size)
{
    uint32_t sum = 0;

    for(int i = 0; i < size; i++)
        sum += *((((char *)heap) + i));

    return sum;
}

struct block_t *get_last_used_block(void)
{
    BLOCK *ptr = heap.head;
    while(ptr->next != NULL)
        ptr = ptr->next;
    return ptr;
}

size_t heap_get_used_space(void)
{
    if(heap.init == 0)
        return 0;

    size_t used = 0;

    BLOCK *ptr = heap.head;

    while(ptr->next != NULL)
    {
        used += ptr->data_size;
        used += BLOCK_SIZE;
        ptr = ptr ->next;
    }

    return used;
}

size_t heap_get_largest_used_block_size(void)
{
    if(heap.init == 0)
        return 0;

    size_t largest = 0;
    BLOCK *ptr = heap.head;

    while(ptr != NULL)
    {
        if(ptr->used)
        {
            if(ptr->data_size > largest)
                largest = ptr->data_size;
        }
        ptr = ptr->next;
    }
    return largest;
}

uint64_t heap_get_used_blocks_count(void)
{
    if(heap.init == 0)
        return 0;

    uint64_t count = 0;
    BLOCK *ptr = heap.head;

    while(ptr != NULL)
    {
        if(ptr->used)
            count++;
        ptr = ptr->next;
    }

    return count;
}

size_t heap_get_free_space(void)
{
    if(heap.init == 0)
        return 0;

    size_t size = 0;
    BLOCK *ptr = heap.head;

    while(ptr != NULL)
    {
        if(!ptr->used)
            size += ptr->data_size;
        ptr = ptr->next;
    }

    return size;
}

size_t heap_get_largest_free_area(void)
{
    if(heap.init == 0)
        return 0;

    size_t size = 0;
    BLOCK *ptr = heap.head;

    while(ptr != NULL)
    {
        if(!ptr->used)
        {
            if(ptr->data_size > size)
                size = ptr->data_size;
            ptr = ptr->next;
        }
    }

    return size;   
}

uint64_t heap_get_free_gaps_count(void)
{
    if(heap.init == 0)
        return 0;

    uint64_t count = 0;
    BLOCK *ptr = heap.head;

    while(ptr != NULL)
    {
        if(!ptr->used)
            count++;
        ptr = ptr->next;
    }

    return count;
}

void heap_dump_debug_information(void)
{
    if(!heap.init)
    {
        printf("Heap is not initialized\n");
        return;
    }
    BLOCK *curr = heap.head;
    while(curr != NULL)
    {
        printf("Adress: %p | Prev adress: %p | Size: %u | Checksum: %u | Used: ", curr, curr->prev, (unsigned int)curr->data_size, (unsigned int)curr->checksum);
        if(curr->used)
            printf("Yes\n");
        else
            printf("No\n");
        
        curr = curr->next;
    }
    printf("Total size: %u | Free size: %u | Total blocks: %u | Biggest block: %u\n", (unsigned int)heap.total_size, (unsigned int)heap.free_size, (unsigned int)heap.block_count, (unsigned int)heap_get_largest_used_block_size());
}

enum pointer_type_t get_pointer_type(const const void* pointer)
{
    if(pointer == NULL)
        return pointer_null;

    if(pointer < (void*)&heap || heap.init == 0)
        return pointer_out_of_heap;

    BLOCK *curr = heap.head;

    while(curr != NULL)
    {
        if(pointer >= (void*)curr && pointer < (void*)(curr+1))
            return pointer_control_block;
        else if(pointer == (void*)(curr + 1) && curr->used)
            return pointer_valid;
        else if(pointer >= (void*)(curr + 1) && (uint8_t*)pointer < (uint8_t*)(curr+1) + curr->data_size)
        {
            if(curr->used)
                return pointer_inside_data_block;
            else
                return pointer_unallocated;
        }
        curr = curr->next;
    }
    return pointer_out_of_heap;
}

void* heap_get_data_block_start(const void* pointer)
{
    if(get_pointer_type(pointer) != pointer_inside_data_block && get_pointer_type(pointer) != pointer_valid)
        return NULL;
    if(get_pointer_type(pointer) == pointer_valid)
        return (void*)pointer;
    void *res = (void*)pointer;
    int found = 0;
    while(!found)
    {
        if(get_pointer_type(res) == pointer_control_block && get_pointer_type(res + 1) != pointer_control_block)
        {
            found = 1;
            BLOCK *block = (BLOCK *)res;
            res = (void *)(block - 1);
        }
        if(!found)
            res--;
    }
    return res;
}

size_t heap_get_block_size(const const void *memblock)
{
    BLOCK *block = (BLOCK *)memblock;
    if(get_pointer_type(block--) != pointer_valid)
        return 0;
    return block->data_size;
}

void heap_reset()
{
    if(heap.init == 0)
        return;
    if(heap.block_count != 1)
        return;
    if(heap.total_size == PAGE_SIZE)
        return;
    size_t to_delete = heap.head->data_size;
    size_t single_block = PAGE_SIZE - BLOCK_SIZE;
    to_delete -= single_block;
    custom_sbrk(-to_delete);
    heap.total_size = PAGE_SIZE;
    heap.free_size = single_block;
    heap.head->data_size = single_block;
    heap.checksum = 0;
    heap.checksum = calculate_checksum(&heap, ((&heap) + 1));
}

void heap_destroy()
{
    if(heap.init == 0)
        return;
    size_t destroy = heap.total_size;
    custom_sbrk(-destroy);
}