#include "drivers/VGA.h"
#include "drivers/PIT.h"
#include "interrupts/IDT.h"
#include "interrupts/exceptions.h"
#include "drivers/IO.h"

#define HALT __asm__ __volatile__("hlt")


char* vga_main = (char*)0xB8000;

void panic(const char* const PANIC_MESSAGE) {
    clearScreen(&vga_main, 0x4, 0xFE);
    kputs(PANIC_MESSAGE, &vga_main, 2);
    __asm__ __volatile__("cli; hlt");
}


static void _test() {
    panic("AH");
}

__attribute__((interrupt)) void timer_irq_stub(int_frame_t* frame) {
    outportb(0x20, 0x20);
}


static void IRQ_clear_mask(unsigned char IRQline) {
    unsigned short port;
    unsigned char value;
 
    if(IRQline < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        IRQline -= 8;
    }
    value = inportb(port) & ~(1 << IRQline);
    outportb(port, value);        
}

int _start() {
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);

    idt_install();
     // Setup exceptions.
    set_idt_entry(0x0, div_0_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x4, overflow_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x5, bre_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x6, invld_opcode_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x7, dev_na_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x8, double_fault_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0x9, cso_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0xA, invld_tss_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0xB, seg_np_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0xC, ss_fault_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0xD, gp_fault_ex, TRAP_GATE_FLAGS);
    set_idt_entry(0xF, float_ex, TRAP_GATE_FLAGS);

    unsigned int divisor = 1193182 / 50;
    outportb(0x43, 0x34);             // Command byte.
    outportb(0x40, divisor & 0xFF);   // Divisor low byte.
    outportb(0x40, divisor >> 8);     // Divisor high byte.
    outportb(0x21, 0xFF);

    set_idt_entry(0x20, timer_irq_stub, INT_GATE_FLAGS);
    IRQ_clear_mask(0x0);
    __asm__ __volatile__("sti");

    return 0;
}
