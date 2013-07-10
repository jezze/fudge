#define GDT_ACCESS_ACCESSED             (0x01 << 0)
#define GDT_ACCESS_RW                   (0x01 << 1)
#define GDT_ACCESS_DC                   (0x01 << 2)
#define GDT_ACCESS_EXECUTE              (0x01 << 3)
#define GDT_ACCESS_ALWAYS1              (0x01 << 4)
#define GDT_ACCESS_RING0                (0x00 << 5)
#define GDT_ACCESS_RING1                (0x01 << 5)
#define GDT_ACCESS_RING2                (0x02 << 5)
#define GDT_ACCESS_RING3                (0x03 << 5)
#define GDT_ACCESS_PRESENT              (0x01 << 7)

#define GDT_FLAG_32BIT                  (0x01 << 6)
#define GDT_FLAG_GRANULARITY            (0x01 << 7)

enum gdt_index
{

    GDT_INDEX_KCODE                     = 0x01,
    GDT_INDEX_KDATA                     = 0x02,
    GDT_INDEX_UCODE                     = 0x03,
    GDT_INDEX_UDATA                     = 0x04,
    GDT_INDEX_TSS                       = 0x05

};

struct gdt_descriptor
{

    unsigned short limit0;
    unsigned short base0;
    unsigned char base1;
    unsigned char access;
    unsigned char limit1;
    unsigned char base2;

} __attribute__((packed));

struct gdt_pointer
{

    unsigned short limit;
    struct gdt_descriptor *descriptors;

} __attribute__((packed));

unsigned short gdt_set_descriptor(struct gdt_pointer *pointer, enum gdt_index index, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags);
void gdt_init_pointer(struct gdt_pointer *pointer, unsigned int count, struct gdt_descriptor *descriptors);
