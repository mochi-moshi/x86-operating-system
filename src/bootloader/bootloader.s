BITS 16

SECTION .text
_start:
    mov byte [drive], dl
    mov di, partition
    mov cx, 4
.load_partition_info:
    movsd
    loop .load_partition_info
load_2nd_stage_and_superblock:
    mov eax, 0
    mov edx, 1
    mov cx, 3
    mov bx, end_of_bootsector
    call read_sectors
load_block_group_table:
    mov edx, dword [superblock+20] ; Block address of superblock
    inc edx ; Block address of block group table
    mov ecx, dword [superblock+24] ; log2(Block size) - 10 
    inc ecx ; Block size base = 2 sectors
    shld eax, edx, cl ; High Block address in sectors
    shl edx, cl       ; Low  Block address in sectors
    shl cx, 2         ;      Block size    in sectors
    mov bx, superblock+1024 ; Assuming block size < 3E sectors
    shr bx, 4
    mov es, bx
    mov bx, 0
    call read_sectors
find_root_inode:
    mov eax, 1 ; root inode number - 1
    mov edx, 0
    mov ebx, dword [superblock+40] ; inodes per block group
    div ebx
    shl eax, 5 ; 32b block group descriptor
    mov si, Hello_Message
    call print
    cli
stall:
    hlt
    jmp stall
; ds:si - C-string to print
print:
    push si
    push ax
    mov ah, 0x0E
.loop:
    lodsb
    int 0x10
    test al, al
    jnz .loop
    pop ax
    pop si
    ret

; eax:edx - lba address to start read at (eax upper 4 bytes, edx lower 4 bytes)
; cx      - number of sectors to read
; es:bx   - segment and offset to read to
; Input register are not saved
read_sectors:
    push si
    mov word [disk_parameters.sector_cnt], cx
    mov word [disk_parameters.offset], bx
    mov word [disk_parameters.segment], es
    add edx, dword [partition.lba_first]
    jnc .no_overflow
    inc eax
.no_overflow:
    mov dword [disk_parameters.lba], edx
    mov dword [disk_parameters.lba+4], eax
    mov si, disk_parameters
    mov cx, 3
    mov dl, byte [drive]
.retry:
    mov ah, 0x42
    int 0x13
    jnc .good_read
    loop .retry
    mov si, disk_read_err
    call print
    cli
.hang:
    hlt
    jmp .hang
.good_read:
    pop si
    ret
disk_read_err: db "Disk read err", 0
disk_parameters: db 0x10, 0
    .sector_cnt: dw 0
    .offset:     dw 0
    .segment:    dw 0
    .lba:        dq 0
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
drive: db 0
Hello_Message: db "Hello there!", 0xA, 0xD, 0
times 510-($-$$) db 0
dw 0xAA55
end_of_bootsector:
times 512*2-($-$$) db 0
end_of_second_stage:
superblock:
