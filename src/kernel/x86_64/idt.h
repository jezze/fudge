#define IDT_FLAG_TYPE64INT              0x0E
#define IDT_FLAG_TYPE64TRAP             0x0F
#define IDT_FLAG_RING1                  0x20
#define IDT_FLAG_RING2                  0x40
#define IDT_FLAG_RING3                  0x60
#define IDT_FLAG_PRESENT                0x80

struct idt_descriptor
{

    unsigned char base0;
    unsigned char base1;
    unsigned char selector0;
    unsigned char selector1;
    unsigned char ist;
    unsigned char flags;
    unsigned char base2;
    unsigned char base3;
    unsigned char base4;
    unsigned char base5;
    unsigned char base6;
    unsigned char base7;
    unsigned char zero0;
    unsigned char zero1;
    unsigned char zero2;
    unsigned char zero3;

};

struct idt_pointer
{

    unsigned char limit0;
    unsigned char limit1;
    unsigned char base0;
    unsigned char base1;
    unsigned char base2;
    unsigned char base3;
    unsigned char base4;
    unsigned char base5;
    unsigned char base6;
    unsigned char base7;

};

void idt_setdescriptor(struct idt_pointer *pointer, unsigned short index, void (*callback)(void), unsigned short selector, unsigned char flags);
void idt_init(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors);
