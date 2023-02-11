BITS 16

SECTION .text
  ; Relocate
  mov si, 0x7C00
  mov di, 0x600
  mov cx, 128
  load_loop:
    movsd
    loop load_loop
  jmp word [jmp_to]
jmp_to: dw _start
_start:
  ; Find bootable partition
  mov si, partition_table_start
  find_boot_partition:
    cmp byte [si], 0x80
    je found_boot_partition
    add si, (partition_table_start-partition.end)
    cmp si, partition_table_end
    jne find_boot_partition 
    mov si, no_bootable_medium
    mov cx, no_bootable_medium.end-no_bootable_medium
    jmp error
  found_boot_partition:
    mov ax, 0
    int 0x13
    mov bx, si
    mov ah, 2
    mov al, 1
    mov ch, [bx+partition.chs_first.ch-partition]
    mov cl, [bx+partition.chs_first.cl-partition]
    mov dh, [bx+partition.chs_first.dh-partition]
    mov bx, 0
    mov es, bx
    mov bx, 0x7c00
    int 0x13
    jnc loaded_partition
    call print_al
    mov al, ah
    call print_al
    mov si, error_reading_partition
    mov cx, error_reading_partition.end-error_reading_partition
    jmp error
  loaded_partition:
    mov al, ah
    call print_al
    mov al, [0x7c00]
    call print_al
    mov si, di
    mov dl, [di]
    mov eax, 0
    mov ebx, 0
    mov ecx, 0
    mov edx, 0
    add di, partition.status-partition
    mov di, 0
    jmp 0x7c00
    found: db "Loaded partition"
    found.end:
error:
  mov ah, 0x0e
  .loop:
    lodsb
    int 0x10
    loop .loop
  cli
  .loop2:
    hlt
    jmp .loop2
print_al:
    push ds
    push bx
    push ax
    push ax
    mov bx, 0
    mov ds, bx
    mov ah, 0x0e
    mov bx, hex_to_ascii
    and al, 0xF0
    shr al, 4
    xlatb
    int 0x10
    pop ax
    mov ah, 0x0e
    and al, 0xF
    xlatb
    int 0x10
    pop ax
    pop bx
    pop ds
    ret

hex_to_ascii: db "0123456789ABCDEF"
no_bootable_medium: db "No bootable medium"
no_bootable_medium.end:
error_reading_partition: db "Error while reading partition"
error_reading_partition.end:
times 440-($-$$) db 0
disk_sig:
  dd 0
  dw 0
partition_table_start:
partition:
  .status: db 0
  .chs_first.dh: db 0
  .chs_first.cl: db 0 ; high 2 bits are for cylinder
  .chs_first.ch: db 0
  .type: db 0
  .chs_last.dh: db 0
  .chs_last.cl: db 0
  .chs_last.ch: db 0
  .lba_first: dd 0
  .size_sec: dd 0
  .end:
times 510-($-$$) db 0
partition_table_end:
dw 0xAA55