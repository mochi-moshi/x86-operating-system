SECTIONS
{
    . = 0x1500;
    .text : {
        *(.text);
    }
    .data : {
        *(.data);
    }
    .bss : {
        *(.bss);
    }
}
