#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H
#include "stddef.h"
#include "physical_memory_manager.h"

#define VMM_PAGES_PER_TABLE 1024
#define VMM_TABLES_PER_DIRECTORY 1024
#define VMM_PAGE_SIZE 4096

typedef uint32_t pt_entry_t __attribute__((aligned (4)));
typedef uint32_t pd_entry_t __attribute__((aligned (4)));

#define VMM_PD_INDEX(vaddr) ((uint32_t)(vaddr) >> 22)
#define VMM_PT_INDEX(vaddr) (((uint32_t)(vaddr) >> 12) & 0x3FF)
#define VMM_PAGE_PHYSICAL_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)
#define VMM_SET_ATTRIBUTE(entry, attr) (*entry |= attr)
#define VMM_CLEAR_ATTRIBUTE(entry, attr) (*entry &= ~attr)
#define VMM_TEST_ATTRIBUTE(entry, attr) (*entry & attr)
#define VMM_SET_FRAME(entry, addr) (*entry = (*entry & ~0x7FFFF000) | (uint32_t)(addr))

#define VMM_PTE_PRESENT       0x01
#define VMM_PTE_READ_WRITE    0x02
#define VMM_PTE_USER          0x04
#define VMM_PTE_WRITE_THROUGH 0x08
#define VMM_PTE_CACHE_DISABLE 0x10
#define VMM_PTE__ACCESSED     0x20
#define VMM_PTE_DIRTY         0x40
#define VMM_PTE_PAT           0x80
#define VMM_PTE_GLOBAL        0x100
#define VMM_PTE_FRAME         0x7FFFF000

#define VMM_PDE_PRESENT       0x01
#define VMM_PDE_READ_WRITE    0x02
#define VMM_PDE_USER          0x04
#define VMM_PDE_WRITE_THROUGH 0x08
#define VMM_PDE_CACHE_DISABLE 0x10
#define VMM_PDE__ACCESSED     0x20
#define VMM_PDE_DIRTY         0x40       // 4MB entry only
#define VMM_PDE_PAGE_SIZE     0x80       // 0 = 4KB page, 1 = 4MB page
#define VMM_PDE_GLOBAL        0x100      // 4MB entry only
#define VMM_PDE_PAT           0x2000     // 4MB entry only
#define VMM_PDE_FRAME         0x7FFFF000

typedef struct __attribute__((packed)) {
        pt_entry_t entries[VMM_PAGES_PER_TABLE];
} page_table_t;

typedef struct __attribute__((packed)) {
        pd_entry_t entries[VMM_TABLES_PER_DIRECTORY];
} page_directory_t;

typedef struct {
    page_directory_t *current_directory;
    memory_region_t* regions[0]; // Ends with null ptr
} vmm_info_t;


static vmm_info_t *__vmm_info;

static inline pt_entry_t *vmm_get_pt_entry(page_table_t *pt, const ptr_t vaddr) {
    return (pt) ? &pt->entries[VMM_PT_INDEX(vaddr)]:0;
}
static inline pd_entry_t *vmm_get_pd_entry(page_directory_t *pd, const ptr_t vaddr) {
    return (pd) ? &pd->entries[VMM_PD_INDEX(vaddr)]:0;
}
static inline pt_entry_t *vmm_get_page(const ptr_t addr) {
    pd_entry_t *entry = vmm_get_pd_entry(__vmm_info->current_directory, addr);
    page_table_t *table = (page_table_t *) VMM_PAGE_PHYSICAL_ADDRESS(entry);
    return vmm_get_pt_entry(table, addr);
}

static ptr_t vmm_allocate_blocks(const size_t count) {
    for(memory_region_t **region = __vmm_info->regions; *region; region++) {
        ptr_t block = pmm_allocate_blocks(*region, count);
        if(block) return block;
    }
    return 0;
}
static void vmm_free_blocks(const ptr_t address, const size_t size) {
    for(memory_region_t **region = __vmm_info->regions; *region; region++) {
        if((*region)->start_address <= address && address <= (*region)->end_address) {
            pmm_free_blocks(*region, address, size);
            break;
        }
    }
}

static ptr_t vmm_allocate_page(pt_entry_t *page) {
    ptr_t block = vmm_allocate_blocks(1);
    if(block) {
        VMM_SET_FRAME(page, block);
        VMM_SET_ATTRIBUTE(page, VMM_PTE_PRESENT);
        return block;
    }
    return 0;
}
static void  vmm_free_page(pt_entry_t *page) {
    ptr_t address = (ptr_t)VMM_PAGE_PHYSICAL_ADDRESS(page);
    if(address) {
        vmm_free_blocks(address, 1);
    }
    VMM_CLEAR_ATTRIBUTE(page, VMM_PTE_PRESENT);
}

static bool_t vmm_set_page_directory(page_directory_t *pd) {
    if(!pd) return false;
    __vmm_info->current_directory = pd;
    __asm__ __volatile__ ("mov %0, %%cr3" : : "r"(__vmm_info->current_directory));
    return true;
}
static inline void vmm_flush_table_entry(ptr_t address) {
    __asm__ __volatile__ ("cli; invlpg (%0); sti" : : "r"(address));
}
static bool_t vmm_map_page(ptr_t physical_address, ptr_t virtual_address) {
    pd_entry_t *entry = vmm_get_pd_entry(__vmm_info->current_directory, virtual_address);
    
    if(!VMM_TEST_ATTRIBUTE(entry, VMM_PDE_PRESENT)) {
        page_table_t *table = (page_table_t *)vmm_allocate_blocks(1);
        if(!table) return false;
        memset(table, 0, sizeof(page_table_t));
        VMM_SET_ATTRIBUTE(entry, VMM_PDE_PRESENT);
        VMM_SET_ATTRIBUTE(entry, VMM_PDE_READ_WRITE);
        VMM_SET_FRAME(entry, table);
    }

    page_table_t *table = (page_table_t*)VMM_PAGE_PHYSICAL_ADDRESS(entry);
    pt_entry_t *page = vmm_get_pt_entry(table, virtual_address);
    VMM_SET_ATTRIBUTE(page, VMM_PTE_PRESENT);
    VMM_SET_FRAME(page, physical_address);
    return true;
}
static void vmm_unmap_page(ptr_t vaddr) {
    pt_entry_t *page = vmm_get_page(vaddr);
    VMM_SET_FRAME(page, 0);
    VMM_CLEAR_ATTRIBUTE(page, VMM_PTE_PRESENT);
}

static bool_t vmm_init(memory_region_t **regions, size_t count) {
    for(memory_region_t **region = regions; *region && region - regions < count; region++) {
        __vmm_info = pmm_allocate_blocks(*region, 1);
        if(__vmm_info) break;
    }
    if(!__vmm_info) return false;
    memset(__vmm_info, 0, sizeof(vmm_info_t)+sizeof(ptr_t)*count);
    for(size_t i = 0; i < count; i++)
        __vmm_info->regions[i] = regions[i];
    
    page_directory_t *dir = (page_directory_t *)vmm_allocate_blocks(3);
    if(!dir) return false;

    memset(dir, 0, sizeof(page_directory_t));
    for(size_t i = 0; i < VMM_TABLES_PER_DIRECTORY; i++)
        dir->entries[i] = VMM_PDE_READ_WRITE;

    page_table_t *table = (page_table_t *)vmm_allocate_blocks(1);
    if(!table) return false;
    page_table_t *table_3gb = (page_table_t *)vmm_allocate_blocks(1);
    if(!table_3gb) return false;
    memset(table, 0, sizeof(page_table_t));
    memset(table_3gb, 0, sizeof(page_table_t));

    // Direct map lower memory    
    for(size_t i = 0, frame = 0; i < VMM_PAGES_PER_TABLE; i++, frame += VMM_PAGE_SIZE) {
        pt_entry_t *page = vmm_get_pt_entry(table_3gb, (ptr_t)frame);
        VMM_SET_ATTRIBUTE(page, VMM_PTE_PRESENT);
        VMM_SET_ATTRIBUTE(page, VMM_PTE_READ_WRITE);
        VMM_SET_FRAME(page, frame);
    }

    pd_entry_t *entry = vmm_get_pd_entry(dir, 0);
    VMM_SET_ATTRIBUTE(entry, VMM_PDE_PRESENT);
    VMM_SET_ATTRIBUTE(entry, VMM_PDE_READ_WRITE);
    VMM_SET_FRAME(entry, table_3gb);

    vmm_set_page_directory(dir);

    return true;
}

static inline void vmm_enter(void) {
    uint32_t cr0;
    __asm__ __volatile__ ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000001;
    __asm__ __volatile__ ("mov %0, %%cr0" : : "r"(cr0));
}

#endif
