#define ARCH_GDTADDRESS                 0x1000
#define ARCH_GDTDESCRIPTORS             256
#define ARCH_IDTADDRESS                 0x2000
#define ARCH_IDTDESCRIPTORS             256
#define ARCH_TSSDESCRIPTORS             1
#define ARCH_KERNELCODEADDRESS          0x00100000
#define ARCH_KERNELCODESIZE             0x00300000
#define ARCH_KERNELSTACKADDRESS         0x00400000
#define ARCH_KERNELSTACKSIZE            0x00004000
#define ARCH_MMUKERNELADDRESS           0x00800000
#define ARCH_MMUKERNELCOUNT             32
#define ARCH_MMUTASKADDRESS             0x00820000
#define ARCH_MMUTASKCOUNT               4
#define ARCH_TASKCODEADDRESS            0x01000000
#define ARCH_TASKCODESIZE               0x00080000
#define ARCH_TASKSTACKADDRESS           0x80000000
#define ARCH_TASKSTACKSIZE              0x00008000

struct arch_context
{

    struct core core;
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
