BITS 16

org 0x7c00
_start:
  mov ah, 0x0E
  mov al, 'A'
  int 0x10
  cli
_stop:
  hlt
  jmp _stop

times 440-($-$$) db 0
disk_sig:
  dd 0
  dw 0
partition_1:
  .status: db 0
  .chs_first.head: db 0
  .chs_first.csect: db 0 ; high 2 bits are for cylinder
  .chs_first.cylin: db 0
  .type: db 0
  .chs_last.head: db 0
  .chs_last.csect: db 0
  .chs_last.cylin: db 0
  .lba_first: dd 0
  .size_sec: dd 0
partition_2:
  .status: db 0
  .chs_first.head: db 0
  .chs_first.csect: db 0 ; high 2 bits are for cylinder
  .chs_first.cylin: db 0
  .type: db 0
  .chs_last.head: db 0
  .chs_last.csect: db 0
  .chs_last.cylin: db 0
  .lba_first: dd 0
  .size_sec: dd 0
partition_3:
  .status: db 0
  .chs_first.head: db 0
  .chs_first.csect: db 0 ; high 2 bits are for cylinder
  .chs_first.cylin: db 0
  .type: db 0
  .chs_last.head: db 0
  .chs_last.csect: db 0
  .chs_last.cylin: db 0
  .lba_first: dd 0
  .size_sec: dd 0
partition_4:
  .status: db 0
  .chs_first.head: db 0
  .chs_first.csect: db 0 ; high 2 bits are for cylinder
  .chs_first.cylin: db 0
  .type: db 0
  .chs_last.head: db 0
  .chs_last.csect: db 0
  .chs_last.cylin: db 0
  .lba_first: dd 0
  .size_sec: dd 0
dw 0xAA55