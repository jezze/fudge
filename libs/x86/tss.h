enum tss_index
{

    TSS_INDEX_DEFAULT                   = 0x00

};

struct tss_descriptor_ring
{

    unsigned int esp;
    unsigned int ss;

};

struct tss_descriptor_general
{

    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;

};

struct tss_descriptor_segment
{

    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;

};

struct tss_descriptor
{

    unsigned int previous;
    struct tss_descriptor_ring ring0;
    struct tss_descriptor_ring ring1;
    struct tss_descriptor_ring ring2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    struct tss_descriptor_general general;
    struct tss_descriptor_segment segment;
    unsigned int ldt;
    unsigned int iopb;

};

struct tss_pointer
{

    unsigned short limit;
    struct tss_descriptor *descriptors;

};

void tss_set_descriptor(struct tss_pointer *pointer, enum tss_index index, unsigned int selector, unsigned int stack);
void tss_init_pointer(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors);
