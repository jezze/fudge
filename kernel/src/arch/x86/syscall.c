#include <runtime.h>
#include <syscall.h>
#include <arch/x86/isr.h>
#include <arch/x86/syscall.h>

static unsigned int (*routines[SYSCALL_TABLE_SLOTS])(struct runtime_task *task, void *stack);

static void register_routine(unsigned int index, unsigned int (*routine)(struct runtime_task *task, void *stack))
{

    routines[index] = routine;

}

static void handle_interrupt(struct runtime_task *task, struct isr_registers *registers)
{

    if (!routines[registers->extra])
    {

        registers->general.eax = 0;

        return;

    }

    registers->general.eax = routines[registers->extra](task, (void *)registers->interrupt.esp);

}

void syscall_setup()
{

    register_routine(SYSCALL_INDEX_OPEN, syscall_open);
    register_routine(SYSCALL_INDEX_CLOSE, syscall_close);
    register_routine(SYSCALL_INDEX_READ, syscall_read);
    register_routine(SYSCALL_INDEX_WRITE, syscall_write);
    register_routine(SYSCALL_INDEX_EXECUTE, syscall_execute);
    register_routine(SYSCALL_INDEX_EXIT, syscall_exit);
    register_routine(SYSCALL_INDEX_WAIT, syscall_wait);
    register_routine(SYSCALL_INDEX_LOAD, syscall_load);
    register_routine(SYSCALL_INDEX_UNLOAD, syscall_unload);
    register_routine(SYSCALL_INDEX_ATTACH, syscall_attach);
    register_routine(SYSCALL_INDEX_DETACH, syscall_detach);
    register_routine(SYSCALL_INDEX_MOUNT, syscall_mount);

    isr_register_routine(ISR_INDEX_SYSCALL, handle_interrupt);

}

