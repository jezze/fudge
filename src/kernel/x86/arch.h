struct arch_context
{

    struct container *container;
    struct task *task;
    unsigned int ip;
    unsigned int sp;

};

void arch_setinterrupt(unsigned char index, void (*callback)(void));
void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int count);
unsigned int arch_call(unsigned int index, void *stack);
struct arch_context *arch_schedule(struct cpu_general *general, unsigned int ip, unsigned int sp);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_setup(struct service_backend *backend);
