#ifndef VIRTUAL_MEMORY_MANAGER_H
#define VIRTUAL_MEMORY_MANAGER_H
#define "stddef.h"

#define VMM_PAGES_PER_TABLE 1024
#define VMM_TABLES_PER_DIRECTORY 1024
#define VMM_PAGE_SIZE 4096

typedef uint32_t pt_entry_t __attribute__((aligned (4)));
typedef uint32_t pd_entry_t __attribute__((aligned (4)));

#define VMM_PD_INDEX(vaddr) ((vaddr) >> 22)
#define VMM_PT_INDEX(vaddr) (((vaddr) >> 12) & 0x3FF)
#define VMM_PAGE_PHYSICAL_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)
#define VMM_SET_ATTRIBUTE(entry, attr) (*entry |= attr)
#define VMM_CLEAR_ATTRIBUTE(entry, attr) (*entry &= ~attr)
#define VMM_TEST_ATTRIBUTE(entry, attr) (*entry & attr)
#define VMM_SET_FRAME(entry, addr) (*entry = (*entry & ~0x7FFFF000) | addr)

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

//typedef struct __attribute__((packed)) {
//        vmm_pt_entry_t entries[VMM_PAGES_PER_TABLE];
//} vmm_page_table_t;
typedef vmm_pt_entry_t vmm_page_table_t[VMM_PAGES_PER_TABLE] __attribute__((packed));

//typedef struct __attribute__((packed)) {
//        vmm_pd_entry_t entries[VMM_TABLES_PER_DIRECTORY];
//} vmm_page_directory_t;
typedef vmm_pd_entry_t vmm_page_directory_t[VMM_TABLES_PER_DIRECTORY] __attribute__((packed));

#endif
