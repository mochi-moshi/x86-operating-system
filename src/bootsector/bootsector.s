BITS 16

SECTION .text
  ; Relocate
  mov si, 0x7C00
  mov di, 0x600
  mov cx, 128
  load_loop:
    movsd
    loop load_loop
  jmp word [_start_loc]
_start:
  ; Find bootable partition
  mov si, partition_table_start
  find_boot_partition:
    cmp byte [si], 0x80
    je found_boot_partition
    add si, (partition.end - partition)
    cmp si, partition_table_end
    jne find_boot_partition 
    mov si, no_bootable_medium
    mov cx, no_bootable_medium.end-no_bootable_medium
    jmp error
  found_boot_partition:
    mov ax, 0
    int 0x13
    mov bx, si
    mov eax, dword [bx+partition.lba_first - partition]
    mov dword [drive_parameters.lba], eax
    mov ax, 0x4200
    mov si, drive_parameters
    int 0x13
    jnc loaded_partition
    mov si, error_reading_partition
    mov cx, error_reading_partition.end-error_reading_partition
    jmp error
  loaded_partition:
    movzx esi, bx
    mov eax, 0
    mov ebx, 0
    mov ecx, 0
    mov edx, 0
    mov edi, 0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov dl, byte [si]
    jmp 0x7c00
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
_start_loc: dw _start
drive_parameters: db 0x10, 0, 1, 0, 0, 0x7c, 0, 0
            .lba: db 0,0,0,0,0,0,0,0
no_bootable_medium: db "No bootable medium"
no_bootable_medium.end:
error_reading_partition: db "Error while reading partition"
error_reading_partition.end:
found: db "Loaded partition"
found.end:
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
