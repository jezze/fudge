#define IDT_ENTRY_SLOTS                 256

#define IDT_FLAG_TYPE32TASK             (0x05 << 0)
#define IDT_FLAG_TYPE16INT              (0x06 << 0)
#define IDT_FLAG_TYPE16TRAP             (0x07 << 0)
#define IDT_FLAG_TYPE32INT              (0x0E << 0)
#define IDT_FLAG_TYPE32TRAP             (0x0F << 0)
#define IDT_FLAG_STORAGE                (0x01 << 4)
#define IDT_FLAG_RING0                  (0x00 << 5)
#define IDT_FLAG_RING1                  (0x01 << 5)
#define IDT_FLAG_RING2                  (0x02 << 5)
#define IDT_FLAG_RING3                  (0x03 << 5)
#define IDT_FLAG_PRESENT                (0x01 << 7)

enum idt_index
{

    IDT_INDEX_DE                        = 0x00,
    IDT_INDEX_DB                        = 0x01,
    IDT_INDEX_NI                        = 0x02,
    IDT_INDEX_BP                        = 0x03,
    IDT_INDEX_OF                        = 0x04,
    IDT_INDEX_BR                        = 0x05,
    IDT_INDEX_UD                        = 0x06,
    IDT_INDEX_NM                        = 0x07,
    IDT_INDEX_DF                        = 0x08,
    IDT_INDEX_CO                        = 0x09,
    IDT_INDEX_TS                        = 0x0A,
    IDT_INDEX_NP                        = 0x0B,
    IDT_INDEX_SS                        = 0x0C,
    IDT_INDEX_GP                        = 0x0D,
    IDT_INDEX_PF                        = 0x0E,
    IDT_INDEX_MF                        = 0x10,
    IDT_INDEX_AC                        = 0x11,
    IDT_INDEX_MC                        = 0x12,
    IDT_INDEX_XM                        = 0x13,
    IDT_INDEX_SYSCALL                   = 0x80

};

struct idt_entry
{

    unsigned short baseLow;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short baseHigh;

} __attribute__((packed));

struct idt_pointer
{

    unsigned short limit;
    struct idt_entry *base;

} __attribute__((packed));

void idt_set_entry(struct idt_entry *entry, void (*callback)(), unsigned short selector, unsigned char flags);
struct idt_pointer *idt_setup_pointer();
