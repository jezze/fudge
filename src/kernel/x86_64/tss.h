struct tss_descriptor_stack
{

    unsigned int low;
    unsigned int high;

};

struct tss_descriptor
{

    unsigned int reserved0;
    struct tss_descriptor_stack rsp0;
    struct tss_descriptor_stack rsp1;
    struct tss_descriptor_stack rsp2;
    struct tss_descriptor_stack reserved1;
    struct tss_descriptor_stack ist[7];
    struct tss_descriptor_stack reserved2;
    unsigned short reserved3;
    unsigned short iopb;

};

struct tss_pointer
{

    unsigned short limit;
    struct tss_descriptor *descriptors;

};

void tss_setdescriptor(struct tss_pointer *pointer, unsigned short index, unsigned long stack);
void tss_init(struct tss_pointer *pointer, unsigned int count, struct tss_descriptor *descriptors);
