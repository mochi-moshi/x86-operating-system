BITS 16

SECTION .text
;;; For now assuming DOS 4.0 EBPB
jmp short _start
nop
BPB:
    .OEM: times 8 db ' '
    .BPS: dw 0
    .SPC: db 0
    .RSC: dw 0
    .FAT: db 0
    .RDT: dw 0
    .TSC: dw 0
    .MDT: db 0
    .SPF: dw 0
    .SPT: dw 0
    .HED: dw 0
    .HSC: dd 0
    .LTS: dd 0
    .DRN: db 0
    .RSV: db 0
    .SIG: db 0
    .VID: dd 0
    .VLA: times 11 db ' '
    .SID: times 8  db ' '
_start:
    mov byte [BPB.DRN], dl
    ; Read second stage into memory after
    add cx, word [BPB.RSC]
    dec cx ; bootloader included
    mov bx, si
    xor eax, eax
    mov edx, dword [bx+8]
    inc edx
    xor bx, bx
    mov es, bx
    mov bx, end_of_bootsector
    call read_sectors
load_root_directory:
    ; Read first sector of Root Directory table into memory after second stage
    mov bx, si
    mov ecx, dword [bx+8]
    movsx ebx, word [BPB.RSC]
    add ecx, ebx
    xor eax, eax
    mov al, byte [BPB.FAT]
    mov bx, word [BPB.SPF]
    xor edx, edx
    mul bx
    shl edx, 16
    or edx, eax
    add edx, ecx
    mov cx, 1
    mov bx, end_of_second_stage
    xor eax, eax
    call read_sectors 
    jmp second_stage 

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
    mov dword [disk_parameters.lba], edx
    mov dword [disk_parameters.lba+4], eax
    mov si, disk_parameters
    mov cx, 3
    mov dl, byte [BPB.DRN]
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
times 510-($-$$) db 0
dw 0xAA55
end_of_bootsector:
Hello_Message: db "Hello there!", 0xA, 0xD, 0

second_stage:
    mov si, Hello_Message
    call print
    mov ax, 0xE0A
    int 0x10
    mov al, 0xD
    int 0x10
    mov si, end_of_second_stage
    mov cx, 11
    call print_c
stall:
    jmp stall
print_c:
    push si
    push ax
    mov ah, 0x0e
.loop:
    lodsb
    int 0x10
    loop .loop
    pop ax
    pop si
    ret
times (4*512)-($-$$) db 0
end_of_second_stage:
