#define TSS_ENTRY_SLOTS                 1

enum tss_index
{

    TSS_INDEX_NULL                      = 0x00

};

struct tss_entry
{

    unsigned int previous;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;

} __attribute__((packed));

void tss_set_entry(enum tss_index index, unsigned int selector, unsigned int stack);
void tss_setup(unsigned short selector, unsigned int stack);
