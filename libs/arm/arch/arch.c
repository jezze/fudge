#include <kernel.h>
#include <kernel/error.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <kernel/task.h>
#include <kernel/scheduler.h>
#include <kernel/container.h>
#include <kernel/kernel.h>
#include "cpu.h"
#include "arch.h"
#include "kmi.h"
#include "pic.h"
#include "reg.h"
#include "uart.h"

static struct task task;

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
    ascii_write_value(num, 32, ip, 16, 0);
    uart_puts(num);
    uart_puts(" ");
    uart_puts("SP: ");
    ascii_write_value(num, 32, sp, 16, 0);
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

