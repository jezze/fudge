struct syscall_registers
{

    struct {unsigned int edi; unsigned int esi; unsigned int ebp; unsigned int esp; unsigned int ebx; unsigned int edx; unsigned int ecx; unsigned int eax;} general;
    struct {unsigned int eip; unsigned int cs; unsigned int eflags; unsigned int esp; unsigned int ss;} interrupt;

};

void syscall_routine();
unsigned short syscall_interrupt(struct syscall_registers *registers);
void syscall_setup_arch(struct runtime_task *task, unsigned short selector);
