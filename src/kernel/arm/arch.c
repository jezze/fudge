#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "kmi.h"
#include "pic.h"
#include "reg.h"
#include "uart.h"
#include "arch.h"

#define CTRL_SIZE_32            0x02
#define CTRL_INT_ENABLE         (1 << 5)
#define CTRL_MODE_PERIODIC      0x40
#define CTRL_ENABLE             0x80

#define REG_LOAD        0x00
#define REG_VALUE       0x01
#define REG_CTRL        0x02
#define REG_INTCLR      0x03
#define REG_INTSTAT     0x04
#define REG_INTMASK     0x05
#define REG_BGLOAD      0x06

#define ARM4_XRQ_RESET   0x00
#define ARM4_XRQ_UNDEF   0x01
#define ARM4_XRQ_SWINT   0x02
#define ARM4_XRQ_ABRTP   0x03
#define ARM4_XRQ_ABRTD   0x04
#define ARM4_XRQ_RESV1   0x05
#define ARM4_XRQ_IRQ     0x06
#define ARM4_XRQ_FIQ     0x07

void timer_setup(void)
{

    reg_write32(0x13000000, 0x00FFFFFF);
    reg_write32(0x13000008, CTRL_SIZE_32 | CTRL_INT_ENABLE | CTRL_MODE_PERIODIC | CTRL_ENABLE);
    reg_write32(0x1300000C, ~0);
    pic_enableirq(PIC_IRQ_TIMER0);

}

extern unsigned int arch_x_swi;
extern unsigned int arch_x_irq;
extern unsigned int arch_x_fiq;

void arch_swi(void)
{

    uart_puts("SWI\n");

}

void arch_irq(void)
{

    unsigned int status = reg_read32(0x14000000);

    uart_puts("IRQ\n");

    if (status == PIC_IRQ_UART0)
    {

        reg_write32(0x16000044, 1);
        uart_puts("  UART0\n");

    }

    if (status == PIC_IRQ_KEYBOARD)
    {

        unsigned int value = reg_read32(0x18000008);

        uart_puts("  KEYBOARD\n");

        if (value == 0x1C)
            uart_puts("  Enter was pressed\n");

    }

    if (status == PIC_IRQ_TIMER0)
    {

        reg_write32(0x1300000C, 1);
        uart_puts("  TIMER0\n");

    }

}

void arch_fiq(void)
{

    uart_puts("FIQ\n");

}

static void xrqinstall(unsigned int index, void *addr)
{

    unsigned int value = 0xEA000000 | (((unsigned int)addr - (8 + (4 * index))) >> 2);

    buffer_copy((void *)(index * 4), &value, 4);

}

void arch_setup(void)
{

    xrqinstall(ARM4_XRQ_RESET, &arch_x_swi);
    xrqinstall(ARM4_XRQ_UNDEF, &arch_x_swi);
    xrqinstall(ARM4_XRQ_SWINT, &arch_x_swi);
    xrqinstall(ARM4_XRQ_ABRTP, &arch_x_swi);
    xrqinstall(ARM4_XRQ_ABRTD, &arch_x_swi);
    xrqinstall(ARM4_XRQ_RESV1, &arch_x_swi);
    xrqinstall(ARM4_XRQ_IRQ, &arch_x_irq);
    xrqinstall(ARM4_XRQ_FIQ, &arch_x_fiq);
    pic_setup();
    uart_setup();
    timer_setup();
    kmi_setup();

    for (;;);

}

