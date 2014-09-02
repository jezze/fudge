enum gdt_access
{

    GDT_ACCESS_ACCESSED                 = 0x01,
    GDT_ACCESS_RW                       = 0x02,
    GDT_ACCESS_DC                       = 0x04,
    GDT_ACCESS_EXECUTE                  = 0x08,
    GDT_ACCESS_ALWAYS1                  = 0x10,
    GDT_ACCESS_RING1                    = 0x20,
    GDT_ACCESS_RING2                    = 0x40,
    GDT_ACCESS_RING3                    = 0x60,
    GDT_ACCESS_PRESENT                  = 0x80

};

enum gdt_flag
{

    GDT_FLAG_32BIT                      = 0x40,
    GDT_FLAG_GRANULARITY                = 0x80

};

enum gdt_index
{

    GDT_INDEX_KCODE                     = 0x01,
    GDT_INDEX_KDATA                     = 0x02,
    GDT_INDEX_UCODE                     = 0x03,
    GDT_INDEX_UDATA                     = 0x04,
    GDT_INDEX_TLINK                     = 0x05

};

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

unsigned short gdt_setdescriptor(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, enum gdt_access access, enum gdt_flag flags);
void gdt_initpointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors);
