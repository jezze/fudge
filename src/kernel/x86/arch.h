struct arch_context
{

    struct task *task;
    unsigned int ip;
    unsigned int sp;

};

struct arch_context *arch_getcontext(void);
void arch_setmap(unsigned char index, unsigned int paddress, unsigned int vaddress, unsigned int count);
struct arch_context *arch_schedule(struct cpu_general *general, unsigned int ip, unsigned int sp);
unsigned short arch_resume(struct arch_context *context, struct cpu_general *general, struct cpu_interrupt *interrupt);
void arch_leave(unsigned short code, unsigned short data, unsigned int ip, unsigned int sp);
void arch_setup(struct service_backend *backend);
