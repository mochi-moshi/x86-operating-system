SECTIONS
{
    . = 0x1500;
    .text : {
        *entry.o;
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
    /DISCARD/ : {
        *(.eh_frame);
        *(.note*);
    }
}
