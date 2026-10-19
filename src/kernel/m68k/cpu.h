#define CPU_SR_SUPERVISOR               0x2000
#define CPU_SR_IPL                      0x0700
#define CPU_SR_CCR                      0x001F
#define CPU_TC_ENABLE                   0x8000
#define CPU_FORMAT_VECTOR               0x0FFF

union cpu_register
{

    unsigned int value;
    void *reference;

};

struct cpu_general
{

    union cpu_register d0;
    union cpu_register d1;
    union cpu_register d2;
    union cpu_register d3;
    union cpu_register d4;
    union cpu_register d5;
    union cpu_register d6;
    union cpu_register d7;
    union cpu_register a0;
    union cpu_register a1;
    union cpu_register a2;
    union cpu_register a3;
    union cpu_register a4;
    union cpu_register a5;
    union cpu_register a6;

};

/* the layout isr.s saves on the kernel stack: the user stack pointer and the registers, then the frame the cpu pushed; ea, ssw and fa only exist in an access fault's frame (format 7) */
struct cpu_frame
{

    union cpu_register usp;
    struct cpu_general general;
    unsigned short sr;
    unsigned int pc;
    unsigned short format;
    unsigned int ea;
    unsigned short ssw;
    unsigned short wb3s;
    unsigned short wb2s;
    unsigned short wb1s;
    unsigned int fa;

};

unsigned int cpu_getroot(void);
void cpu_setroot(unsigned int value);
void cpu_settc(unsigned int value);
void cpu_setvbr(void *value);
void cpu_flushtlb(void);
void cpu_halt(void);
void cpu_leave(struct cpu_frame *frame);
