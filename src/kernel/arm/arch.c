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

static struct core core0;

static unsigned int spawn(struct task *task, void *stack)
{

    unsigned int id = kernel_loadtask(task, ARCH_TASKSTACKVIRTUAL);

    return id;

}

static unsigned int despawn(struct task *task, void *stack)
{

    task_setstate(task, TASK_STATE_KILLED);

    return 0;

}

static struct core *coreget(void)
{

    return &core0;

}

static void coreassign(struct task *task)
{

    list_add(&core0.tasks, &task->item);

}

static void debugnum(unsigned int value, unsigned int base)
{

    char num[ASCII_NUMSIZE];

    buffer_clear(num, ASCII_NUMSIZE);
    ascii_wvalue(num, ASCII_NUMSIZE, value, base, 0);
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
    core_init(&core0, 0, ARCH_KERNELSTACKPHYSICAL + ARCH_KERNELSTACKSIZE);
    kernel_setup(ARCH_MAILBOXPHYSICAL, ARCH_MAILBOXSIZE);
    kernel_setcallback(coreget, coreassign);
    abi_setup(spawn, despawn);
    uart_puts("Loop forever...\n");

    for (;;);

}

