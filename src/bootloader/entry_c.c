#include "entry_c.h"
#include "asm.h"

extern ptr_t kernel_loader_begin;
extern ptr_t kernel_loader_end;

static GDT_entry_t GDT[3];
static IDT_t IDT;
static memory_region_t *Regions[8];

__attribute__((noreturn))
__attribute__((section (".text.first")))
void kernel_entry() {
    kernel_pass_t* kernel_pass;
    __asm__ __volatile__ ("mov %0, %%eax" : "=r"(kernel_pass));

    clear_screen();
    print("Loading System...\n");

    smap_t* smap = kernel_pass->smap;
    ptr_t current_start = 0;
    size_t current_size = 0;
    uint8_t current_region = 0;
    for(uint16_t i = 0; i < smap->length; i++) {
        smap_entry_t entry = smap->entries[i];
        if(entry.type == 1) {
            if(current_size == 0) current_start = (ptr_t)entry.base_address;
            current_size += (size_t)entry.length;
        } else if(current_size) {
            Regions[current_region++] = pmm_create_region(current_start, current_start+current_size);
            current_size = 0;
        }
        print("Type:         ");
        switch(entry.type) {
            case 1:
                print("Usable");
                break;
            case 2:
                print("Reserved");
                break;
            case 3:
                print("ACPI reclaimable");
                break;
            case 4:
                print("ACPI NVS");
                break;
            case 5:
                break;
            default:
                print_dword(entry.type);
                break;
        }
        print("\nBase Address: ");
        print_qword(entry.base_address);
        print("\nLength:       ");
        print_qword(entry.length);
        putch('\n');
    }

    memset(GDT, 0, sizeof(GDT));
    memset(IDT, 0, sizeof(IDT));

    print("Loading GDT...");
    gdt_set_entry(&GDT[1], 0xFFFFF, 0, 0x9A, 0xC);
    gdt_set_entry(&GDT[2], 0xFFFFF, 0, 0x92, 0xC);
    gdt_load(GDT, 3);
    print("Done\nLoading IDT...");

    idt_set_entry(&IDT[0], default_int, 0x8, IDT_32BIT_INT, IDT_PRESENT);
    for(uint8_t i = 1; i > 0; ++i)
        idt_set_entry(&IDT[i], default_int, 0x8, IDT_32BIT_INT, IDT_PRESENT);
    print("Done\nLoading PIC...");

    pic_init(IDT, 0x20, 0x28);
    idt_load(IDT);
    sti();
    print("Done\nLoading Memory Manager...");

    size_t count = 0;
    for(uint8_t count = 0; Regions[count]; count++) {
        memory_region_t *region = Regions[count];
        if(region->start_address <= kernel_loader_begin)
            pmm_deinitialize_memory_region(&region->blocks_desc, (size_t)kernel_loader_begin, kernel_loader_end-kernel_loader_begin);
    }
    vmm_init(Regions, count);
    vmm_enter();
    print("Done\nEntered Virtual Memory Mode\n");

    partition_info_t *partition = kernel_pass->partition;
    drive_init(partition->status, partition->lba_first, partition->size);
    panic();
}

__attribute__((interrupt))
void default_int(ptr_t stack) { }

