#include <kernel.h>
#include <kernel/kernel.h>
#include "arch.h"
#include "cpu.h"
#include "kmi.h"
#include "pic.h"
#include "reg.h"
#include "uart.h"

static struct runtime_task task;

/*
static void enable_interrupts()
{

    reg_write32(PIC_REG_IRQ_ENABLECLR, PIC_IRQ_KEYBOARD);

    reg_write32(KMI_REG_KBD_CONTROL, KMI_CONTROL_INTRX | KMI_CONTROL_INTTX);

    reg_write32(PIC_REG_IRQ_ENABLESET, reg_read32(PIC_REG_IRQ_ENABLESET) | PIC_IRQ_KEYBOARD);

}
*/

static void enter_usermode(unsigned int ip, unsigned int sp)
{

    char num[32];

    uart_puts("USERMODE\n");
    uart_puts("IP: ");
    string_write_num(num, ip, 16);
    uart_puts(num);
    uart_puts(" ");
    uart_puts("SP: ");
    string_write_num(num, sp, 16);
    uart_puts(num);
    uart_puts("\n");
    kmi_setup();

}

void isr_undefined()
{

    uart_puts("INTERRUPT\n");

    for (;;);

}

void arch_setup()
{

    uart_puts("Fudge Console Text\n");
    kernel_setup(&task, 0, 0);
    enter_usermode(task.registers.ip, task.registers.sp);

}

