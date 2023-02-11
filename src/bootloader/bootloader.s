BITS 16

SECTION .text
;;; For now assuming DOS 4.0 EBPB
jmp short _start
BPB:
    .OEM: db "        "
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
    .VLA: db "           "
    .SID: db "        "
_start:
    cli
stall:
    hlt
    jmp stall

times 510-($-$$) db 0
dw 0xAA55
