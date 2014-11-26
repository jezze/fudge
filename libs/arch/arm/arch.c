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

#define ARCH_CONTAINERS                 32
#define ARCH_TASKS                      64
#define ARCH_TASK_STACKLIMIT            0x80000000

static struct arch_container
{

    struct container base;

} containers[ARCH_CONTAINERS];

static struct arch_task
{

    struct task base;

} tasks[ARCH_TASKS];

static struct
{

    struct container *container;
    struct task *task;

} current;

static unsigned int spawn(struct container *self, struct task *task, void *stack)
{

    return 0;

}

static unsigned int exit(struct container *self, struct task *task, void *stack)
{

    return 0;

}

void arch_undefined()
{

    uart_puts("ISR UNDEFINED\n");

    for (;;);

}

void arch_reset()
{

    uart_puts("ISR RESET\n");

    for (;;);

}

void arch_syscall()
{

    uart_puts("ISR SYSCALL\n");

    for (;;);

}

void arch_irq()
{

    uart_puts("ISR IRQ\n");

    for (;;);

}

void arch_fiq()
{

    uart_puts("ISR FIQ\n");

    for (;;);

}

static void setupbasic()
{

}

static void setupcontainer(struct arch_container *container, unsigned int i)
{

    container_init(&container->base, spawn, exit);

}

static struct container *setupcontainers()
{

    unsigned int i;

    for (i = 0; i < ARCH_CONTAINERS; i++)
        setupcontainer(&containers[i], i);

    return &containers[0].base;

}

static void setuptask(struct arch_task *task, unsigned int i)
{

    task_init(&task->base, 0, ARCH_TASK_STACKLIMIT);

    scheduler_register_task(&task->base);

}

static struct task *setuptasks()
{

    unsigned int i;

    for (i = 0; i < ARCH_TASKS; i++)
        setuptask(&tasks[i], i);

    return &tasks[0].base;

}

void arch_setup()
{

    setupbasic();
    kernel_setup();

    current.container = setupcontainers();
    current.task = setuptasks();

    pic_setup();
    uart_setup();
    uart_puts("Fudge Console\n");

/*
    memory_clear(num, 32);
    uart_puts(num);
    x = cpu_get_cpsr();
    ascii_wvalue(num, 32, x, 16, 0);
    uart_puts(num);
    uart_puts("\n");
*/

    scheduler_use(current.task);

    uart_puts("Loop forever...\n");

    for (;;);

}

