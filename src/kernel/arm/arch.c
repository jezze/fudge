#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "kmi.h"
#include "pic.h"
#include "reg.h"
#include "uart.h"
#include "arch.h"

extern void halt(void);

static void debugnum(unsigned int value, unsigned int base)
{

    char num[32];

    buffer_clear(num, 32);
    cstring_write_value(num, 32, value, base, 0, 0);
    uart_puts(num);
    uart_puts("\n");

}

void arch_undefined(void)
{

    uart_puts("ISR UNDEFINED\n");

    for (;;);

}

void arch_reset(void)
{

    uart_puts("ISR RESET\n");

    for (;;);

}

__attribute__ ((interrupt("SWI"))) void arch_swi(void)
{

    register unsigned int addr __asm__ ("r14");

    addr -= 4;

    uart_puts("ISR SWI: ");
    debugnum(*((unsigned int *)(addr)) & 0x00FFFFFF, 10);

}

__attribute__ ((interrupt("IRQ"))) void arch_irq(void)
{

    uart_puts("ISR IRQ\n");

}

__attribute__ ((interrupt("FIQ"))) void arch_fiq(void)
{

    uart_puts("ISR FIQ\n");

}

void pic_do(void)
{

    unsigned int *mmio = (unsigned int *)0x14000000;

    mmio[0x02] = (1 << 5) | (1 << 6) | (1 << 7);

}

void timer_do(void)
{

    unsigned int *mmio = (unsigned int *)0x13000000;

    mmio[0x00] = 0xffffff;
    mmio[0x06] = 0xffffff;
    mmio[0x02] = 0x80 | 0x40 | 0x02 | 0x00 | (1 << 5);
    /*
    mmio[0x02] = ~0;
    */

}

void arch_setup(void)
{

    pic_setup();
    pic_do();

    /*
    timer_do();
    */

    uart_puts("Fudge Console\n");
    /*swi_test();*/

    resource_setup();
    kernel_setup(ARCH_KERNELSTACKPHYSICAL, ARCH_KERNELSTACKSIZE, ARCH_MAILBOXPHYSICAL, ARCH_MAILBOXSIZE);
    abi_setup();
    uart_puts("Loop forever...\n");

    for (;;);

}

