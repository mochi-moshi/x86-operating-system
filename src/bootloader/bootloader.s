BITS 16

inode_loc equ 0x500

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
    mov cl, byte [superblock+24] ; log2(Block size) - 10 
    inc cl ; Block size base = 2 sectors
    mov byte [block_to_sector], cl
    mov bx, 1
    shl bx, cl
    mov word [sectors_per_block], bx
    mov eax, 512
    mul bx
    mov ebx, superblock+1024
    add ebx, eax
    mov dword [inode_table_location], ebx
load_block_group_table:
    mov edx, dword [superblock+20] ; Block address of superblock
    inc edx ; Block address of block group table
    mov bx, superblock+1024 ; Assuming block size < 3E sectors
    shr bx, 4
    push es
    mov es, bx
    mov bx, 0
    call read_block
    pop es
    mov eax, 0 ; loading root inode table
    call load_inode_table
load_root_inode_data:
    mov ebx, dword [inode_table_location]
    movzx ecx, word [superblock+88]
    add ebx, ecx
    mov dword [tmp_pointer], ebx
    ; Assuming valid and the root directory is not hashed
    mov edx, [ebx+40] ; block pointer 0
    mov bx, inode_loc
    call read_block
search_for_boot_file:
    mov bx, inode_loc
    add bx, word [bx+4] ; skip .
    add bx, word [bx+4] ; skip ..
    mov si, bx
    add si, 8
    movsx cx, byte [bx+6]
    call printc
    mov ax, 0xe0a
    int 0x10
    mov ax, 0xe0d
    int 0x10
    mov eax, dword [bx]
    call print_eax
    dec eax
    movzx ecx, word [superblock+88]
    mul ecx
    mov ebx, dword [inode_table_location]
    add ebx, eax
    mov dword [tmp_pointer], ebx
    mov edx, [ebx+40]
    mov bx, inode_loc
    call read_block
    mov si, inode_loc
    mov cx, 25
    call printc
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
; ds:si - string to print
; cx - length of string
printc:
    push si
    push ax
    mov ah, 0x0E
.loop:
    lodsb
    int 0x10
    loop .loop
    pop ax
    pop si
    ret
; edx - block address
; es:bx - location to load at
read_block:
    push eax
    push edx
    push bx
    push cx
    push es
    mov cl, byte [block_to_sector]
    mov eax, 0
    shld eax, edx, cl ; High Block address in sectors
    shl edx, cl       ; Low  Block address in sectors
    mov cx, word [sectors_per_block] 
    call read_sectors
    pop es
    pop cx
    pop bx
    pop edx
    pop eax
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
tmp_pointer: dd 0
sectors_per_block: dw 0
block_to_sector: db 0
drive: db 0
Hello_Message: db "Hello there!", 0xA, 0xD, 0
times 510-($-$$) db 0
dw 0xAA55
end_of_bootsector:
print_al:
    push ebx
    push ax
    mov ebx, hex_to_ascii
    call _print_al
    pop ax
    pop ebx
    ret 
_print_al:
    push ax
    shr al, 4
    xlatb
    mov ah, 0xe
    int 0x10
    pop ax
    and al, 0xf
    xlatb
    mov ah, 0xe
    int 0x10
    ret
print_ax:
    push ebx
    push ax
    mov ebx, hex_to_ascii
    call _print_ax
    pop ax
    pop ebx
    ret
_print_ax:
    push ax
    shr ax, 8
    call _print_al
    pop ax
    call _print_al
    ret
print_eax:
    push ebx
    push eax
    push eax
    shr eax, 16
    mov ebx, hex_to_ascii
    call _print_ax
    pop eax
    call _print_ax
    pop eax
    pop ebx
    ret
; eax - inode table number
load_inode_table:
    pusha
    push es
    mov word [current_inode_table], ax 
    shl eax, 5 ; 32b block group descriptor
    add eax, superblock+1024 ; goto block descriptor
    mov ebx, eax
    mov edx, dword [ebx+8] ; inode table block address
    mov cx, word [sectors_per_block]
    mov ebx, dword [inode_table_location]
    shr ebx, 4
    mov es, bx
    mov ebx, dword [inode_table_location]
    and ebx, 0xF
    call read_block
    pop es
    popa
    ret
inode_table_location: dd 0
current_inode_table: dw 0
hex_to_ascii: db "0123456789abcdef?????"
times 512*2-($-$$) db 0
end_of_second_stage:
superblock:
