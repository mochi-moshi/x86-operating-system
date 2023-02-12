BITS 16

SECTION .text
_start:
    mov byte [drive], dl
    mov word [partition], si
    mov bx, si
    mov edx, [bx+8]
    inc edx
    mov bx, end_of_bootsector
    mov cx, 3
    call read_sectors
    pop dx
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
drive: db 0
partition: dw 0
Hello_Message: db "Hello there!", 0xA, 0xD, 0
times 510-($-$$) db 0
dw 0xAA55
end_of_bootsector:
