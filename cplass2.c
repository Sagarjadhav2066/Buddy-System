#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#define TOTAL_HEAP 1024
#define ALIGN 8
#define ALIGN_SIZE(s) (((s) + (ALIGN - 1)) & ~(ALIGN - 1))

typedef struct MemoryBlock {
    size_t block_size;
    bool is_free;
    struct MemoryBlock *next_block;
    struct MemoryBlock *prev_block;
} MemoryBlock;

MemoryBlock *memory_heap = NULL;
MemoryBlock *free_blocks;

void setupHeap() {
    memory_heap = (MemoryBlock*) malloc(TOTAL_HEAP);
    if (!memory_heap) {
        printf("Heap setup failed!\n");
        exit(EXIT_FAILURE);
    }
    free_blocks = (MemoryBlock*) memory_heap;
    free_blocks->block_size = TOTAL_HEAP - sizeof(MemoryBlock);
    free_blocks->is_free = true;
    free_blocks->next_block = NULL;
    free_blocks->prev_block = NULL;
}

void *getMemory(size_t requested_size) {
    requested_size = ALIGN_SIZE(requested_size);
    MemoryBlock *current_block = free_blocks;
    while (current_block) {
        if (current_block->is_free && current_block->block_size >= requested_size) {
            if (current_block->block_size >= requested_size + sizeof(MemoryBlock) + ALIGN) {
                MemoryBlock *new_block = (MemoryBlock*)((char*)current_block + sizeof(MemoryBlock) + requested_size);
                new_block->block_size = current_block->block_size - requested_size - sizeof(MemoryBlock);
                new_block->is_free = true;
                new_block->next_block = current_block->next_block;
                new_block->prev_block = current_block;
                if (new_block->next_block) {
                    new_block->next_block->prev_block = new_block;
                }
                current_block->block_size = requested_size;
                current_block->next_block = new_block;
            }
            current_block->is_free = false;
            return (char*)current_block + sizeof(MemoryBlock);
        }
        current_block = current_block->next_block;
    }
    return NULL;
}

void combineFreeBlocks() {
    MemoryBlock *current_block = free_blocks;
    while (current_block && current_block->next_block) {
        if (current_block->is_free && current_block->next_block->is_free) {
            current_block->block_size += current_block->next_block->block_size + sizeof(MemoryBlock);
            current_block->next_block = current_block->next_block->next_block;
            if (current_block->next_block) {
                current_block->next_block->prev_block = current_block;
            }
        } else {
            current_block = current_block->next_block;
        }
    }
}

void releaseMemory(void *memory_ptr) {
    if (!memory_ptr) return;
    MemoryBlock *block = (MemoryBlock*)((char*)memory_ptr - sizeof(MemoryBlock));
    block->is_free = true;
    combineFreeBlocks();
}

void showHeap() {
    MemoryBlock *current_block = free_blocks;
    while (current_block) {
        printf("Block: Address = %p, Size = %zu, Free = %s\n", 
               (void*)current_block, current_block->block_size, current_block->is_free ? "Yes" : "No");
        current_block = current_block->next_block;
    }
}

void defragHeap() {
    combineFreeBlocks();
    printf("Heap defragmented!\n");
}

void printMenu() {
    printf("\n--- Memory Allocator Menu ---\n");
    printf("1. Allocate Memory\n");
    printf("2. Free Memory\n");
    printf("3. Defragment Heap\n");
    printf("4. Show Heap State\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    setupHeap();
    int choice;
    void *allocated_memory[10] = {NULL};
    int allocated_count = 0;

    while (1) {
        printMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                if (allocated_count >= 10) {
                    printf("Maximum allocations reached!\n");
                    break;
                }
                size_t size;
                printf("Enter size to allocate: ");
                scanf("%zu", &size);
                void *mem = getMemory(size);
                if (mem) {
                    printf("Memory allocated at address: %p\n", mem);
                    allocated_memory[allocated_count++] = mem;
                } else {
                    printf("Memory allocation failed! Not enough space.\n");
                }
                break;
            }

            case 2: {
                if (allocated_count == 0) {
                    printf("No memory allocated to free!\n");
                    break;
                }
                int index;
                printf("Enter index of memory to free (0-%d): ", allocated_count - 1);
                scanf("%d", &index);
                if (index >= 0 && index < allocated_count && allocated_memory[index]) {
                    printf("Freeing memory at address: %p\n", allocated_memory[index]);
                    releaseMemory(allocated_memory[index]);
                    allocated_memory[index] = NULL;
                } else {
                    printf("Invalid index or memory already freed!\n");
                }
                break;
            }

            case 3: {
                defragHeap();
                break;
            }

            case 4: {
                printf("\nCurrent Heap State:\n");
                showHeap();
                break;
            }

            case 5: {
                printf("Exiting...\n");
                free(memory_heap);
                return 0;
            }

            default: {
                printf("Invalid choice! Please try again.\n");
                break;
            }
        }
    }

    return 0;
}