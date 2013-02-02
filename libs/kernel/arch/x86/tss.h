#define TSS_ENTRY_SLOTS                 1

enum tss_index
{

    TSS_INDEX_NULL                      = 0x00

};

struct tss_entry_ring
{

    unsigned int esp;
    unsigned int ss;

} __attribute__((packed));

struct tss_entry
{

    unsigned int previous;
    struct tss_entry_ring ring0;
    struct tss_entry_ring ring1;
    struct tss_entry_ring ring2;
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
    unsigned short iomap;

} __attribute__((packed));

struct tss_pointer
{

    unsigned short limit;
    struct tss_entry *base;

} __attribute__((packed));

void tss_set_entry(struct tss_pointer *p, enum tss_index index, unsigned int selector, unsigned int stack);
struct tss_pointer *tss_setup_pointer();
