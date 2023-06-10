#define GDT_ACCESS_ACCESSED             0x01
#define GDT_ACCESS_RW                   0x02
#define GDT_ACCESS_DC                   0x04
#define GDT_ACCESS_EXECUTE              0x08
#define GDT_ACCESS_ALWAYS1              0x10
#define GDT_ACCESS_RING1                0x20
#define GDT_ACCESS_RING2                0x40
#define GDT_ACCESS_RING3                0x60
#define GDT_ACCESS_PRESENT              0x80
#define GDT_FLAG_32BIT                  0x40
#define GDT_FLAG_GRANULARITY            0x80

struct gdt_descriptor
{

    unsigned char limit0;
    unsigned char limit1;
    unsigned char base0;
    unsigned char base1;
    unsigned char base2;
    unsigned char access;
    unsigned char limit2;
    unsigned char base3;

};

struct gdt_pointer
{

    unsigned char limit0;
    unsigned char limit1;
    unsigned char base0;
    unsigned char base1;
    unsigned char base2;
    unsigned char base3;

};

unsigned short gdt_getselector(struct gdt_pointer *pointer, unsigned char index);
void gdt_setdescriptor(struct gdt_pointer *pointer, unsigned char index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags);
void gdt_init(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors);
