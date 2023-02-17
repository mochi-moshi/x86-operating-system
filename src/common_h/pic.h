#ifndef PIC_H
#define PIC_H
#include "stddef.h"
#include "asm.h"
#include "idt.h"

#define PIC1_REG_COMMAND_PORT    0x20
#define PIC1_REG_STATUS_PORT     0x20
#define PIC1_REG_DATA_PORT       0x21
#define PIC1_REG_IMR_PORT        0x21

#define PIC2_REG_COMMAND_PORT    0xA0
#define PIC2_REG_STATUS_PORT     0xA0
#define PIC2_REG_DATA_PORT       0xA1
#define PIC2_REG_IMR_PORT        0xA1

#define PIC_OCW2_MASK_EOI        0x20

#define PIC_IRQ_TIMER      0
#define PIC_IRQ_CMOSTIMER  0
#define PIC_IRQ_KEYBOARD   1
#define PIC_IRQ_CGARETRACE 1
#define PIC_IRQ_SERIAL2    3
#define PIC_IRQ_SERIAL1    4
#define PIC_IRQ_AUXILIARY  4
#define PIC_IRQ_PARALLEL2  5
#define PIC_IRQ_FPU        5
#define PIC_IRQ_DISKETTE   6
#define PIC_IRQ_HDC        6
#define PIC_IRQ_PARALLEL1  7

#define PIC_ICW1_IC4_EXPECT          1
#define PIC_ICW1_SNGL                2
#define PIC_ICW1_ADI_CALLINTERVAL8   0
#define PIC_ICW1_ADI_CALLINTERVAL4   4
#define PIC_ICW1_LTIM_EDGETRIGGERED  0
#define PIC_ICW1_LTIM_LEVELTRIGGERED 8
#define PIC_ICW1_INIT                16

#define PIC_ICW4_UPM_MCSMODE     0
#define PIC_ICW4_UPM_86MODE      1
#define PIC_ICW4_AEOI_AUTOEOI    2
#define PIC_ICW4_MS_BUFFERSLAVE  0
#define PIC_ICW4_MS_BUFFERMASTER 4
#define PIC_ICW4_BUF_MODE        8
#define PIC_ICW4_SFNM_NESTEDMODE 16

#define PIC_OCW3_READ_IRR 0xA
#define PIC_OCW3_READ_ISR 0xB

static uint8_t __pic_base0;
static uint8_t __pic_base1;

static inline void pic_disable_irq(uint8_t line) {
    uint8_t port = PIC1_REG_DATA_PORT;
    if(line >= 8) {
        port = PIC2_REG_DATA_PORT;
        line -= 8;
    }
    uint8_t value = inb(port) | (1 << line);
    outb(port, value);
}
static inline void pic_enable_irq(uint8_t line) {
    uint8_t port = PIC1_REG_DATA_PORT;
    if(line >= 8) {
        port = PIC2_REG_DATA_PORT;
        line -= 8;
    }
    uint8_t value = inb(port) & ~(1 << line);
    outb(port, value);
}
static inline void pic_eoi(uint8_t intno) {
    if(intno <= 16) {
        if(intno >= 8) outb(PIC2_REG_COMMAND_PORT, PIC_OCW2_MASK_EOI);
        outb(PIC1_REG_COMMAND_PORT, PIC_OCW2_MASK_EOI);
    } else {
        // TODO: ERROR handling
    }
}
static inline uint16_t pic_get_irr(void) {
    outb(PIC1_REG_COMMAND_PORT, PIC_OCW3_READ_IRR);
    outb(PIC2_REG_COMMAND_PORT, PIC_OCW3_READ_IRR);
    return (inb(PIC2_REG_DATA_PORT) << 8) | inb(PIC1_REG_DATA_PORT);
}
static inline uint16_t pic_get_isr(void) {
    outb(PIC1_REG_COMMAND_PORT, PIC_OCW3_READ_ISR);
    outb(PIC2_REG_COMMAND_PORT, PIC_OCW3_READ_ISR);
    return (inb(PIC2_REG_DATA_PORT) << 8) | inb(PIC1_REG_DATA_PORT);
}
static inline void pic_mask_irq(uint8_t pic, uint8_t mask) {
    if(pic < 2) {
        uint8_t port = (pic == 1)? PIC2_REG_DATA_PORT:PIC1_REG_DATA_PORT;
        outb(port, mask);
    } else {
        // TODO: ERROR handling
    }
}
static inline uint8_t pic_read_data(uint8_t pic) {
    if(pic < 2) {
        uint8_t port = (pic == 1)? PIC2_REG_DATA_PORT:PIC1_REG_DATA_PORT;
        return inb(port);
    } else {
        // TODO: ERROR handling
    }
    return 0;
}
static inline void pic_send_command(uint8_t pic, uint8_t command) {
    if(pic < 2) {
        uint8_t port = (pic == 1)? PIC2_REG_COMMAND_PORT:PIC1_REG_COMMAND_PORT;
        outb(port, command);
    } else {
        // TODO: ERROR handling
    }
}
static inline void pic_send_data(uint8_t pic, uint8_t data) {
    if(pic < 2) {
        uint8_t port = (pic == 1)? PIC2_REG_DATA_PORT:PIC1_REG_DATA_PORT;
        outb(port, data);
    } else {
        // TODO: ERROR handling
    }

}
static inline void pic_set_irq(IDT_t IDT, uint8_t irqno, interrupt_handler_t handler) {
    idt_set_entry(&IDT[__pic_base0+irqno], handler, 0x8, IDT_32BIT_INT, IDT_PRESENT); 
}

__attribute__((interrupt))
static void __pic_irq7_handler(interrupt_stack_t* stack) {

}

static        void pic_init(IDT_t IDT, uint8_t base0, uint8_t base1) {
    __pic_base0 = base0;
    __pic_base1 = base1;
    idt_set_entry(&IDT[base0+7], __pic_irq7_handler, 0x8, IDT_32BIT_INT, IDT_PRESENT);

    pic_send_command(0, PIC_ICW1_IC4_EXPECT | PIC_ICW1_INIT);
    pic_send_command(1, PIC_ICW1_IC4_EXPECT | PIC_ICW1_INIT);

    pic_send_data(0, base0);
    pic_send_data(1, base1);

    pic_send_data(0, 4); // slave on line 2
    pic_send_data(1, 2); // master on line 2

    pic_send_data(0, PIC_ICW4_UPM_86MODE);
    pic_send_data(1, PIC_ICW4_UPM_86MODE);

    pic_send_data(0, 0xFF); // mask all interupts
    pic_send_data(1, 0xFF);
}
#endif
