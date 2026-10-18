#define CPU_STATUS_SIE                  (1UL << 1)
#define CPU_STATUS_SPIE                 (1UL << 5)
#define CPU_STATUS_SPP                  (1UL << 8)
#define CPU_STATUS_SUM                  (1UL << 18)
#define CPU_CAUSE_INTERRUPT             (1UL << 63)
#define CPU_CAUSE_TIMER                 5
#define CPU_CAUSE_EXTERNAL              9
#define CPU_CAUSE_ILLEGAL               2
#define CPU_CAUSE_USERCALL              8
#define CPU_CAUSE_FETCHPAGE             12
#define CPU_CAUSE_LOADPAGE              13
#define CPU_CAUSE_STOREPAGE             15

union cpu_register
{

    unsigned long value;
    void *reference;

};

/* every register but sp, x1 and x3-x31 */
struct cpu_general
{

    union cpu_register ra;
    union cpu_register gp;
    union cpu_register tp;
    union cpu_register t0;
    union cpu_register t1;
    union cpu_register t2;
    union cpu_register s0;
    union cpu_register s1;
    union cpu_register a0;
    union cpu_register a1;
    union cpu_register a2;
    union cpu_register a3;
    union cpu_register a4;
    union cpu_register a5;
    union cpu_register a6;
    union cpu_register a7;
    union cpu_register s2;
    union cpu_register s3;
    union cpu_register s4;
    union cpu_register s5;
    union cpu_register s6;
    union cpu_register s7;
    union cpu_register s8;
    union cpu_register s9;
    union cpu_register s10;
    union cpu_register s11;
    union cpu_register t3;
    union cpu_register t4;
    union cpu_register t5;
    union cpu_register t6;

};

struct cpu_interrupt
{

    union cpu_register sp;
    union cpu_register pc;
    union cpu_register status;

};

/* the layout trap.s saves on the kernel stack */
struct cpu_frame
{

    struct cpu_general general;
    struct cpu_interrupt interrupt;
    unsigned long pad;

};

unsigned long cpu_getsatp(void);
void cpu_setsatp(unsigned long value);
void cpu_flushtlb(void);
unsigned long cpu_gettime(void);
void cpu_settimer(unsigned long time);
void cpu_enableinterrupt(unsigned long mask);
void cpu_disableinterrupt(unsigned long mask);
void cpu_settrap(void (*trap)(void));
void cpu_setstatus(unsigned long value);
void cpu_halt(void);
void cpu_leave(struct cpu_frame *frame);
