BITS 16

SECTION .text
_start:
    mov di, partition
    mov cx, drive-partition+1
    shr cx, 2
    inc cx
.mov:
    movsd 
    loop .mov
load_SMAP_entries:
    mov si, memory_message
    call print

    get_memory_map:
        mov di,  SMAP_entries+2
        xor ebx, ebx
        xor bp,  bp
        mov edx, 'PAMS'
        mov eax, 0xE820
        mov [es:di+20], dword 1
        mov ecx, 24
        int 0x15
        jc .error

        cmp eax, 'PAMS'
        jne .error
        test ebx, ebx
        jz  .error
        jmp .start
    .next_entry:
        mov edx, 'PAMS'
        mov ecx, 24
        mov eax, 0xE820
        int 0x15
    .start:
        jcxz .skip_entry
        mov ecx, [es:di+8]
        or  ecx, [es:di+12]
        jz .skip_entry
    .good_entry:
        inc bp
        add di, 24
    .skip_entry:
        test ebx, ebx
        jz  .done
        jmp .next_entry
    .error:
        stc
    .done:
        mov [SMAP_entries], bp
        clc
load_video_mode:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    mov si, video_message
    call print

    ; setup vbe info
    mov ah, 0x4F
    mov di, vbe_info_block
    int 0x10

    cmp ax, 0x4F
    jne error

    mov ax, word [vbe_info_block.video_mode_pointer]
    mov [offset],    ax
    mov ax, word [vbe_info_block.video_mode_pointer+2]
    mov [t_segment], ax

    mov fs, ax
    mov si, [offset]

    .find_mode:             ; TODO: add resolution selector
        mov dx, [fs:si]
        add si, 2
        mov [offset], si
        mov [mode],   dx

        cmp dx, word 0xFFFF ; end of list
        je end_of_modes

        mov ax, 0x4F01
        mov cx, [mode]
        or cx, 0x4000
        mov di, mode_info_block
        int 0x10

        cmp ax, 0x4F
        jne error

        ;mov dx, [mode_info_block.x_resolution]
        ;shr dx, 4
        ;cmp dx, 0
        ;jl .not_resolution
        ;mov ax, [mode_info_block.y_resolution]
        ;mov cl, 9
        ;div cl
        ;cmp ah, 0
        ;jne .not_resolution

        movsx dx, byte [mode_info_block.bits_per_pixel]

        .not_resolution:
        mov ax, [width]
        cmp ax, [mode_info_block.x_resolution]
        jne .next_mode

        mov ax, [height]
        cmp ax, [mode_info_block.y_resolution]
        jne .next_mode

        mov ax, [bpp]
        cmp al, [mode_info_block.bits_per_pixel]
        jne .next_mode
        jmp load_GDT
    .next_mode:
        mov ax, [t_segment]
        mov fs, ax
        mov si, [offset]
        jmp .find_mode
    error:
        mov si, video_error
        call print
        cli
        hlt
    end_of_modes:
        mov si, veom
        call print
        cli
        hlt
load_GDT:
    cli
    lgdt [ptgdt]
    sti

    ; enable mode
    ;mov ax, 0x4F02
    ;mov bx, [mode]
    ; or bx,  0x4000
    ;xor di, di
    ;int 0x10

    ;mov ax, 0x4F

    ;cmp ax, 0x4F
    ;jne error

    ; enable A20 through keyboard microcontroller
    mov al, 2
    out 0x92, al

enter_protected_mode:
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pre_kern
align 8
ptgdt:
    dw GDT.end - GDT.null - 1
    dd GDT
GDT:
    .null:
        dd 0
        dd 0
    .kernel_code:
        dw 0xFFFF        ; limit low
        dw 0            ; base low
        db 0            ; base mid
        db 10011010b    ; access
        db 11001111b    ; granuality
        db 0            ; base high
    .kernel_data:
        dw 0xFFFF        ; limit low
        dw 0            ; base low
        db 0            ; base mid
        db 10010010b    ; access
        db 11001111b    ; granuality
        db 0            ; base high
    .end:
print:
    mov ah, 0xe
.loop:
    lodsb
    int 0x10
    test al, al
    jnz .loop
    ret
message: db "Hello, World!", 0
memory_message: db "Loading SMAP entries...", 0xA, 0xD, 0
video_message: db "Loading VBE information...", 0xA, 0xD, 0
veom: db "Hit end of video modes", 0
video_error: db "There was an error loading video mode", 0
align 4
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
align 4
inode_table_location: dd 0
current_inode_table: dw 0
         .block_num: dd 0
sectors_per_block: dw 0
block_to_sector: db 0
drive: db 0
align 4
width:      dw 1280 ;800
height:     dw 720 ;600
bpp:        db 32
offset:     dw 0
t_segment:  dw 0
mode:       dw 0
align 4
vbe_info_block: ; 0x8400
    .vbe_signature: db 'VBE2'
    .vbe_version: dw 0x300
    .oem_string_pointer: dd 0
    .capabilities: dd 0
    .video_mode_pointer: dd 0
    .total_memory: dw 0
    .oem_software_rev: dw 0
    .oem_vendor_name_pointer: dd 0
    .oem_product_name_pointer: dd 0
    .oem_product_revision_pointer: dd 0
    .reserved: times 222 db 0
    .oem_data: times 256 db 0
align 4
mode_info_block: ; 0x8600
    .mode_attributes: dw 0
    .window_a_attributes: db 0
    .window_b_attributes: db 0
    .window_granularity: dw 0
    .window_size: dw 0
    .window_a_segment: dw 0
    .window_b_segment: dw 0
    .window_function_pointer: dd 0
    .bytes_per_scanline: dw 0

    .x_resolution: dw 0
    .y_resolution: dw 0
    .x_charsize: db 0
    .y_charsize: db 0
    .number_of_planes: db 0
    .bits_per_pixel: db 0
    .number_of_banks: db 0
    .memory_model: db 0
    .bank_size: db 0
    .number_of_image_pages: db 0
    .reserved1: db 1

    .red_mask_size: db 0
    .red_field_position: db 0
    .green_mask_size: db 0
    .green_field_position: db 0
    .blue_mask_size: db 0
    .blue_field_position: db 0
    .reserved_mask_size: db 0
    .reserved_field_position: db 0
    .direct_color_mode_info: db 0

    .physical_base_pointer: dd 0
    .reserved2: dd 0
    .reserved3: dw 0

    .linear_bytes_per_scan_line: dw 0

    .bank_number_of_image_pages: dw 0
    .linear_number_of_image_pages: db 0
    .linear_red_mask_size: db 0
    .linear_red_field_position: db 0
    .linear_green_mask_size: db 0
    .linear_green_field_position: db 0
    .linear_blue_mask_size: db 0
    .linear_blue_field_position: db 0
    .linear_reserved_mask_size: db 0
    .linear_reserved_field_position: db 0
    .max_pixel_clock: dd 0
    .reserved4: times 190 db 0
align 4
SMAP_entries:
    .number_of_entries: dw 0
    times 510 db 0
align 4
kernel_pass:
    .smap_pointer: dd SMAP_entries
    .vbe_pointer: dd vbe_info_block
    .mode_pointer: dd mode_info_block
    .partition_start: dd partition
    
BITS 32
pre_kern:
    cli

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7FFFF
    mov ebp, esp

    xor eax, eax
    mov eax, kernel_pass
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    xor esi, esi
    xor edi, edi
    call 0x8:kernel_entry
align 4
extern kernel_entry
