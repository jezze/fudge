#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "kmi.h"
#include "pic.h"
#include "reg.h"
#include "uart.h"

#define CONTAINERS                      8
#define TASKS                           64
#define TASKVSTACKLIMIT                 0x80000000

extern void halt(void);

struct atag_header
{

    unsigned int size;
    unsigned int tag;

};

static struct arch_container
{

    struct container base;

} containers[CONTAINERS];

static struct arch_task
{

    struct task base;

} tasks[TASKS];

static struct
{

    struct container *container;
    struct task *task;

} current;

static struct service_backend backend;

static unsigned int backend_read(unsigned int offset, void *buffer, unsigned int count)
{

    return 0;

}

static unsigned int backend_write(unsigned int offset, void *buffer, unsigned int count)
{

    return 0;

}

static unsigned int backend_getphysical(void)
{

    return 0;

}

static unsigned int spawn(struct container *container, struct task *task, void *stack)
{

    return 0;

}

static unsigned int despawn(struct container *container, struct task *task, void *stack)
{

    return 0;

}

static void debugnum(unsigned int value, unsigned int base)
{

    char num[FUDGE_NSIZE];

    memory_clear(num, FUDGE_NSIZE);
    ascii_wvalue(num, FUDGE_NSIZE, value, base, 0);
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

static void setupcontainer(struct arch_container *container, unsigned int i)
{

    container_init(&container->base);

}

static struct container *setupcontainers(void)
{

    unsigned int i;

    for (i = 0; i < CONTAINERS; i++)
        setupcontainer(&containers[i], i);

    return &containers[0].base;

}

static void setuptask(struct arch_task *task, unsigned int i)
{

    task_init(&task->base);

    scheduler_registertask(&task->base);

}

static struct task *setuptasks(void)
{

    unsigned int i;

    for (i = 0; i < TASKS; i++)
        setuptask(&tasks[i], i);

    return &tasks[0].base;

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
    mmio[0x02] = ~0;

}

void arch_setup(void)
{

    pic_setup();
    pic_do();

    /*
    timer_do();
    */

    uart_puts("Fudge Console\n");
    swi_test();

    service_initbackend(&backend, 1000, backend_read, backend_write, backend_getphysical);
    kernel_setup();

    current.container = setupcontainers();
    current.task = setuptasks();

    kernel_setupramdisk(current.container, current.task, &backend);
    kernel_copytask(current.task, current.task);
    kernel_setuptask(current.task, TASKVSTACKLIMIT);
    scheduler_setstatus(current.task, TASK_STATUS_ACTIVE);
    abi_setup(spawn, despawn);

    uart_puts("Loop forever...\n");

    for (;;);

}

