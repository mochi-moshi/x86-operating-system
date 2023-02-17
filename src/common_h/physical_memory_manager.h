#ifndef PHYSICAL_MEMORY_MANAGER_H
#define PHYSICAL_MEMORY_MANAGER_H

#include "stddef.h"
#include "stdlib.h"

#define PMM_BLOCK_SIZE      4096
#define PMM_BLOCKS_PER_BYTE 8

typedef struct  {
    size_t number_of_blocks;
    size_t used_blocks;
    size_t number_of_bytes;
    uint8_t *blocks;
} memory_blocks_t;

typedef uint8_t pmm_block_t[PMM_BLOCK_SIZE];

typedef struct {
    memory_blocks_t blocks_desc;
    ptr_t start_address;
    ptr_t end_address;
    uint8_t blocks[0];
} memory_region_t;


static inline bool_t pmm_test_block(const memory_blocks_t *memory_blocks, const uint32_t address) {
        return memory_blocks->blocks[address/PMM_BLOCKS_PER_BYTE] & (0x80 >> (address % PMM_BLOCKS_PER_BYTE));
}
static inline void pmm_set_block(memory_blocks_t *memory_blocks, const uint32_t address) {
        if(!pmm_test_block(memory_blocks, address)) memory_blocks->used_blocks++;
        memory_blocks->blocks[address/PMM_BLOCKS_PER_BYTE] |= (0x80 >> (address % PMM_BLOCKS_PER_BYTE));
}
static inline void pmm_unset_block(memory_blocks_t *memory_blocks, const uint32_t address) {
        if(pmm_test_block(memory_blocks, address)) memory_blocks->used_blocks--;
        memory_blocks->blocks[address/PMM_BLOCKS_PER_BYTE] &= ~(0x80 >> (address % PMM_BLOCKS_PER_BYTE));
}

static memory_blocks_t pmm_create_blocks_descriptor(ptr_t blocks_location, const size_t size) {
    memory_blocks_t block_manager = { 0 };
    block_manager.number_of_blocks = size / PMM_BLOCK_SIZE;
    block_manager.used_blocks = block_manager.number_of_blocks;
    block_manager.number_of_bytes = block_manager.number_of_blocks / PMM_BLOCKS_PER_BYTE;
    if(block_manager.number_of_blocks % PMM_BLOCKS_PER_BYTE) block_manager.number_of_bytes++;
    block_manager.blocks = blocks_location;
    memset(blocks_location, 0xFF, block_manager.number_of_bytes);
    return block_manager;
}

static void pmm_initialize_memory_region(memory_blocks_t *memory_blocks, const size_t start_address, const size_t size);
// Created a region description at the end of the region
static memory_region_t* pmm_create_region(ptr_t start_address, ptr_t end_address) {
    size_t needed_blocks = (end_address-start_address) / PMM_BLOCK_SIZE;
    end_address = start_address + needed_blocks * PMM_BLOCK_SIZE;
    ptr_t block_map = end_address - needed_blocks / PMM_BLOCKS_PER_BYTE - sizeof(memory_region_t);
    if(needed_blocks % PMM_BLOCKS_PER_BYTE) block_map++;
    memory_region_t* region = block_map - (size_t)(block_map - start_address) % PMM_BLOCK_SIZE;
    region->blocks_desc = pmm_create_blocks_descriptor(region->blocks, needed_blocks * PMM_BLOCK_SIZE);
    region->start_address = start_address;
    region->end_address = start_address + needed_blocks * PMM_BLOCK_SIZE;
    pmm_initialize_memory_region(&region->blocks_desc, 0, (ptr_t)region-start_address);
    return region;
}

static void pmm_initialize_memory_region(memory_blocks_t *memory_blocks, const size_t start_address, const size_t size) {
    uint32_t last_block = (start_address + size);
    last_block += PMM_BLOCK_SIZE - (last_block % PMM_BLOCK_SIZE);
    last_block /= PMM_BLOCK_SIZE;
    for(uint32_t block = start_address / PMM_BLOCK_SIZE; block < last_block; block++) {
        pmm_unset_block(memory_blocks, block);
    }
}
static void pmm_deinitialize_memory_region(memory_blocks_t *memory_blocks, const size_t start_address, const size_t size) {
    uint32_t last_block = ((size_t)start_address + size);
    last_block += PMM_BLOCK_SIZE - (last_block % PMM_BLOCK_SIZE);
    last_block /= PMM_BLOCK_SIZE;
    for(uint32_t block = start_address / PMM_BLOCK_SIZE; block < last_block; block++) {
        pmm_set_block(memory_blocks, block);
    }
}

static ptrdiff_t pmm_find_first_free(const memory_blocks_t *memory_blocks) {
    if(memory_blocks->used_blocks != memory_blocks->number_of_blocks) {
        for(ptrdiff_t i = 0; i < memory_blocks->number_of_blocks; i++) {
            if(memory_blocks->blocks[i / PMM_BLOCKS_PER_BYTE] == 0xFF) {
                i += PMM_BLOCKS_PER_BYTE;
                continue;
            } 
            if(!pmm_test_block(memory_blocks, i)) return i;
        }
    }
    return 0;
}
static ptrdiff_t pmm_find_first_free_run(const memory_blocks_t *memory_blocks, const size_t size) {
    if(size && memory_blocks->used_blocks != memory_blocks->number_of_blocks) {
        size_t current_run = 0;
        for(ptrdiff_t i = 0; i < memory_blocks->number_of_blocks; i++) {
            if(memory_blocks->blocks[i / PMM_BLOCKS_PER_BYTE] == 0xFF) {
                current_run = 0;
                continue;
            }
            if(pmm_test_block(memory_blocks, i)) {
                current_run = 0;
                i += PMM_BLOCKS_PER_BYTE;
                continue;
            }
            if(++current_run == size) return i-current_run+1;
        }
    }
    return 0;
}
static ptr_t pmm_allocate_blocks(memory_region_t *region, const size_t size) {
    if(region->blocks_desc.number_of_blocks-region->blocks_desc.used_blocks < size) return 0;
    ptrdiff_t start = pmm_find_first_free_run(&region->blocks_desc, size);
    if(!start) return 0;
    for(size_t i=0; i < size; i++)
        pmm_set_block(&region->blocks_desc, start+i);
    return (ptr_t)((pmm_block_t*)region->start_address+start);
}
static void pmm_free_blocks(memory_region_t *region, const ptr_t address, const size_t size) {
    for(size_t i=0; i < size; i++)
        pmm_unset_block(&region->blocks_desc, (ptrdiff_t)(address-region->start_address)/PMM_BLOCK_SIZE+i);
}

#endif
