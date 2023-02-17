SECTIONS
{
    . = 0x1500;
    .text : {
        *entry.o;
        kernel_loader_begin = .;
        *(.text.first);
        *(.text);
    }
    .rodata : {
        *(.rodata);
    }
    .data : {
        *(.data);
    }
    .bss : {
        *(.bss);
    }
    kernel_loader_end = .;
    /DISCARD/ : {
        *(.eh_frame);
        *(.note*);
    }
}
