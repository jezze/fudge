struct arch_context
{

    struct task *task;
    unsigned int ip;
    unsigned int sp;

};

struct arch_context *arch_getcontext(void);
void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int count);
void arch_schedule(struct cpu_general *general, struct arch_context *context, unsigned int ip, unsigned int sp);
unsigned short arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(unsigned short code, unsigned short data, unsigned int ip, unsigned int sp);
void arch_setup(struct service_backend *backend);
