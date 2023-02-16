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
find_boot_folder:
    mov si, boot_folder_name
    mov cl, byte [boot_folder_name.size]
    call search_for_entry
    test eax, eax
    jnz load_boot_folder_data
    mov si, boot_folder_not_found
    call print
    jmp halt
load_boot_folder_data:
    mov ebx, eax
    mov eax, dword [bx]
    dec eax
    movsx ecx, word [superblock+88]
    mul ecx ; assuming still in same table
    mov ebx, dword [inode_table_location]
    add ebx, eax
    mov edx, dword [ebx+40] ; pointer 0
    mov bx, inode_loc
    call read_block
find_boot_file:
    mov si, boot_file_name
    mov cl, byte [boot_file_name.size]
    call search_for_entry
    test eax, eax
    jnz load_boot_file
    mov si, boot_file_not_found
    call print
    jmp halt
load_boot_file:
    mov eax, dword [eax]
    dec eax
    xor edx, edx
    movzx ecx, word [superblock+40]
    div ecx
    cmp ax, word [current_inode_table]
    je .no_update
    call load_inode_table
.no_update:
    mov eax, edx
    movsx ecx, word [superblock+88]
    mul ecx
load_boot_file_cnt:
    ; edx:eax - byte offset, ignore edx
    ; Need to check if new block needs to be loaded
    ; mov eax, dword [ebx+28]
    movzx ecx, word [sectors_per_block]
    shl ecx, 9 ; bytes per block
    div ecx
    push edx ; offset of inode into block
    mov edx, eax ; block offset
    add edx, dword [current_inode_table.block_num]
    xor eax, eax
    mov ebx, dword [inode_table_location]
    call read_block
    pop edx
    ; Read up to 6 valid blocks
    add ebx, edx ; Inode entry
    push ebx
    mov eax, 0
    mov cx, 6
valid_block_pointer:
    pop ebx
    mov edx, dword [ebx+40]
    test edx, edx
    jz jmp_to_third_stage
    add ebx, 4
    push ebx
    mov ebx, 0x150
    mov es, ebx
    mov ebx, eax
    call read_block
    add eax, 0x1000
    loop valid_block_pointer
jmp_to_third_stage:
    mov ebx, 0
    mov es, ebx
    mov ebx, 0x1500
    mov si, partition
    jmp ebx
halt:
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
sectors_per_block: dw 0
block_to_sector: db 0
drive: db 0
tmp_pointer: dd 0
disk_read_err: db "Disk read err", 0
times 510-($-$$) db 0
dw 0xAA55
end_of_bootsector:
inode_table_location: dd 0
current_inode_table: dw 0
         .block_num: dd 0
boot_folder_name: db "boot"
           .size: db 4
boot_file_name:   db "entry"
         .size:   db 5
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
    ret
; ds:si - name of entry to file
; cl - length of name
; returns eax = inode number, 0 if not found
search_for_entry:
    pushad
    and cx, 0xFF
    mov ebx, inode_loc
    add bx, word [bx+4] ; skip .
    add bx, word [bx+4] ; skip ..
    mov dx, word [sectors_per_block]
    shl dx, 9 ; bytes in block
    add dx, bx ; end of table
.entry_present:
    mov eax, dword [bx]
    test eax, eax
    jnz .file
.not_used:
    mov ax, bx
    add bx, word [bx+4]
    cmp ax, bx
    je .error ; null entry
    cmp bx, dx
    jl .entry_present
    mov dword [__tmp], 0
    jmp .finished
.error:
    mov si, invalid_directory_entry
    call print
    jmp halt
.file:
    mov al, byte [bx+6]
    cmp al, cl
    jne .not_used
    mov di, bx
    add di, 8
    push si
    push cx
    repe cmpsb
    pop cx
    pop si
    jnz .not_used
    mov dword [__tmp], ebx
.finished:
    popad
    mov eax, dword [__tmp]
    ret
__tmp: dd 0
; edx - block address
; es:bx - location to load at
read_block:
    pushad
    mov cl, byte [block_to_sector]
    mov eax, 0
    shld eax, edx, cl ; High Block address in sectors
    shl edx, cl       ; Low  Block address in sectors
    mov cx, word [sectors_per_block] 
    call read_sectors
    popad
    ret
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
    pushad
    push es
    mov word [current_inode_table], ax 
    shl eax, 5 ; 32b block group descriptor
    add eax, superblock+1024 ; goto block descriptor
    mov ebx, eax
    mov edx, dword [ebx+8] ; inode table block address
    mov dword [current_inode_table.block_num], edx
    mov cx, word [sectors_per_block]
    mov ebx, dword [inode_table_location]
    shr ebx, 4
    mov es, bx
    mov ebx, dword [inode_table_location]
    and ebx, 0xF
    call read_block
    pop es
    popad
    ret
hex_to_ascii: db "0123456789abcdef?????"
boot_folder_not_found: db "boot folder not found",0
boot_file_not_found: db "boot file not found",0
invalid_directory_entry: db "invalid directory entry",0
times 512*2-($-$$) db 0
end_of_second_stage:
superblock:
