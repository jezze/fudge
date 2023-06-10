#define IDT_FLAG_TYPE32TASK             0x05
#define IDT_FLAG_TYPE16INT              0x06
#define IDT_FLAG_TYPE16TRAP             0x07
#define IDT_FLAG_TYPE32INT              0x0E
#define IDT_FLAG_TYPE32TRAP             0x0F
#define IDT_FLAG_STORAGE                0x10
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
    unsigned char zero;
    unsigned char flags;
    unsigned char base2;
    unsigned char base3;

};

struct idt_pointer
{

    unsigned char limit0;
    unsigned char limit1;
    unsigned char base0;
    unsigned char base1;
    unsigned char base2;
    unsigned char base3;

};

void idt_setdescriptor(struct idt_pointer *pointer, unsigned char index, void (*callback)(void), unsigned short selector, unsigned char flags);
void idt_init(struct idt_pointer *pointer, unsigned int count, struct idt_descriptor *descriptors);
