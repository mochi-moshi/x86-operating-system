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
    .end : {
        . = 0x24FC;
        LONG(0x78563412);
    }
    /DISCARD/ : {
        *(.eh_frame);
        *(.note*);
    }
}
