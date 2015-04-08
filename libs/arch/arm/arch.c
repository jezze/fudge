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

extern void halt();

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

static struct vfs_backend backend;

static unsigned int backend_read(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int backend_write(struct vfs_backend *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned long backend_getphysical(struct vfs_backend *self)
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

static void setupcontainer(struct arch_container *container, unsigned int i)
{

    container_init(&container->base);

}

static struct container *setupcontainers()
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

static struct task *setuptasks()
{

    unsigned int i;

    for (i = 0; i < TASKS; i++)
        setuptask(&tasks[i], i);

    return &tasks[0].base;

}

static void debugnum(unsigned int value, unsigned int base)
{

    char num[32];

    memory_clear(num, 32);
    ascii_wvalue(num, 32, value, base, 0);
    uart_puts(num);
    uart_puts("\n");

}

void arch_setup()
{

    pic_setup();
    uart_setup();
    uart_puts("Fudge Console\n");
    debugnum(0x123456, 16);
    vfs_initbackend(&backend, 1000, backend_read, backend_write, backend_getphysical);
    kernel_setup(spawn, despawn);

    current.container = setupcontainers();
    current.task = setuptasks();

    kernel_setupramdisk(current.container, current.task, &backend);
    kernel_copytask(current.task, current.task);
    kernel_setuptask(current.task, TASKVSTACKLIMIT);
    uart_puts("Loop forever...\n");
    halt();

}

