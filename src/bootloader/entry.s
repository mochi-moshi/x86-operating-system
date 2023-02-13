BITS 16

_start:
    mov di, partition
    push es
    push ds
    push ds
    push es
    pop ds
    pop es
    mov cx, drive-partition+1
    shr cx, 2
    inc cx
.mov:
    movsd 
    loop .mov
    pop ds
    pop es
    mov si, message
    call print
    cli
.loop:
    hlt
    jmp .loop
print:
    mov ah, 0xe
.loop:
    lodsb
    int 0x10
    test al, al
    jnz .loop
    ret
message: db "Hello, World!", 0
partition:
    .status: db 0
    .chs_first.dh: db 0
    .chs_first.cl: db 0
    .chs_first.ch: db 0
    .type: db 0
    .chs_last.dh: db 0
    .chs_last.cl: db 0
    .chs_last.ch: db 0
    .lba_first: dd 0
    .size: dd 0
    .end:
inode_table_location: dd 0
current_inode_table: dw 0
         .block_num: dd 0
sectors_per_block: dw 0
block_to_sector: db 0
drive: db 0
