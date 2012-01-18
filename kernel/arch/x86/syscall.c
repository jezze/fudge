#include <kernel/mmu.h>
#include <kernel/runtime.h>
#include <kernel/syscall.h>
#include <kernel/arch/x86/syscall.h>

static void syscall_save_state(struct runtime_task *task, struct syscall_registers *registers)
{

    task->registers.ip = registers->eip;
    task->registers.sp = registers->useresp;
    task->registers.sb = registers->ebp;

}

static void syscall_load_state(struct runtime_task *task, struct syscall_registers *registers)
{

    registers->eip = task->registers.ip;
    registers->useresp = task->registers.sp;
    registers->ebp = task->registers.sb;

}

void syscall_handle(struct syscall_registers *registers)
{

    struct runtime_task *task = runtime_get_running_task();

    syscall_save_state(task, registers);
    registers->eax = syscall_raise(registers->eax, registers->useresp, task);
    syscall_load_state(runtime_get_running_task(), registers);

}

