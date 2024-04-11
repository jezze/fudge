#include "cpu.h"
#include "reg.h"
#include "pic.h"

#include "uart.h"
#include "kmi.h"
#include "timer.h"

#define PIC_REG_IRQ_STATUS              0x14000000
#define PIC_REG_IRQ_RAWSTAT             0x14000004
#define PIC_REG_IRQ_ENABLESET           0x14000008
#define PIC_REG_IRQ_ENABLECLR           0x1400000C
#define PIC_REG_INT_SOFTSET             0x14000010
#define PIC_REG_INT_SOFTCLR             0x14000014
#define PIC_REG_FIQ_STATUS              0x14000020
#define PIC_REG_FIQ_RAWSTAT             0x14000024
#define PIC_REG_FIQ_ENABLESET           0x14000028
#define PIC_REG_FIQ_ENABLECLR           0x1400002C

static void enableirq(void)
{

    cpu_set_cpsr(cpu_get_cpsr() & ~(1 << 7));

}

/*
static void enablefiq(void)
{

    cpu_set_cpsr(cpu_get_cpsr() & ~(1 << 6));

}
*/

void pic_enableirq(unsigned int irq)
{

    reg_write32(PIC_REG_IRQ_ENABLESET, irq);

}

void pic_disableirq(unsigned int irq)
{

    reg_write32(PIC_REG_IRQ_ENABLECLR, irq);

}

void pic_enablefiq(unsigned int fiq)
{

    reg_write32(PIC_REG_FIQ_ENABLESET, fiq);

}

void pic_disablefiq(unsigned int fiq)
{

    reg_write32(PIC_REG_FIQ_ENABLECLR, fiq);

}

void pic_irq(void)
{

    unsigned int status = reg_read32(PIC_REG_IRQ_STATUS);

    if (status == PIC_IRQ_UART0)
    {

        uart_irq();

    }

    if (status == PIC_IRQ_KEYBOARD)
    {

        kmi_kbd_irq();

    }

    if (status == PIC_IRQ_MOUSE)
    {

        kmi_mouse_irq();

    }

    if (status == PIC_IRQ_TIMER0)
    {

        timer_irq();

    }

}

void pic_fiq(void)
{

    uart_puts("FIQ\n");

}

void pic_setup(void)
{

    enableirq();

}

