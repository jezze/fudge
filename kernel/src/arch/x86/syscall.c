#include <isr.h>
#include <runtime.h>
#include <syscall.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static unsigned int (*routines[SYSCALL_TABLE_SLOTS])(struct isr_context *context);

static unsigned int handle_attach(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);
    unsigned int callback = *(unsigned int *)(context->running->registers.sp + 8);

    return syscall_attach(context->running, index, callback);

}

static unsigned int handle_close(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);

    return syscall_close(context->running, index);

}

static unsigned int handle_detach(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);

    return syscall_detach(context->running, index);

}

static unsigned int handle_execute(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);

    return syscall_execute(context->running, index);

}

static unsigned int handle_exit(struct isr_context *context)
{

    return syscall_exit(context->running);

}

static unsigned int handle_load(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);

    return syscall_load(context->running, index);

}

static unsigned int handle_open(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);
    void *buffer = *(char **)(context->running->registers.sp + 8);

    return syscall_open(context->running, index, buffer);

}

static unsigned int handle_read(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);
    unsigned int offset = *(unsigned int *)(context->running->registers.sp + 8);
    unsigned int count = *(unsigned int *)(context->running->registers.sp + 12);
    void *buffer = *(char **)(context->running->registers.sp + 16);

    return syscall_read(context->running, index, offset, count, buffer);

}

static unsigned int handle_unload(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);

    return syscall_unload(context->running, index);

}

static unsigned int handle_wait(struct isr_context *context)
{

    return syscall_wait(context->running);

}

static unsigned int handle_write(struct isr_context *context)
{

    unsigned int index = *(unsigned int *)(context->running->registers.sp + 4);
    unsigned int offset = *(unsigned int *)(context->running->registers.sp + 8);
    unsigned int count = *(unsigned int *)(context->running->registers.sp + 12);
    void *buffer = *(char **)(context->running->registers.sp + 16);

    return syscall_write(context->running, index, offset, count, buffer);

}

static void register_routine(unsigned char index, unsigned int (*routine)(struct isr_context *context))
{

    routines[index] = routine;

}

static void handle_interrupt(struct isr_context *context, struct isr_cpu_registers *registers)
{

    if (!routines[registers->error])
    {

        registers->general.eax = 0;

        return;

    }

    registers->general.eax = routines[registers->error](context);

}

void syscall_init()
{

    register_routine(SYSCALL_INDEX_OPEN, handle_open);
    register_routine(SYSCALL_INDEX_CLOSE, handle_close);
    register_routine(SYSCALL_INDEX_READ, handle_read);
    register_routine(SYSCALL_INDEX_WRITE, handle_write);
    register_routine(SYSCALL_INDEX_EXECUTE, handle_execute);
    register_routine(SYSCALL_INDEX_EXIT, handle_exit);
    register_routine(SYSCALL_INDEX_WAIT, handle_wait);
    register_routine(SYSCALL_INDEX_LOAD, handle_load);
    register_routine(SYSCALL_INDEX_UNLOAD, handle_unload);
    register_routine(SYSCALL_INDEX_ATTACH, handle_attach);
    register_routine(SYSCALL_INDEX_DETACH, handle_detach);

    isr_register_routine(ISR_INDEX_SYSCALL, handle_interrupt);

}

