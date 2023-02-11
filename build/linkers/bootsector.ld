SECTIONS
{
  . = 0x600;
  .text : {
    *(.text.start)
    *(.text);
  }
  .rodata : {
    *(.rodata);
  }
  .bss : {
    *(.bss);
  }
}