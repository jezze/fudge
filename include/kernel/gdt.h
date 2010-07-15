#ifndef GDT_H
#define GDT_H

#define GDT_TABLE_SIZE 6

typedef struct gdt_entry
{

    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t baseMiddle;
    uint8_t access;
    uint8_t granularity;
    uint8_t baseHigh;

} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr
{

    uint16_t limit;
    uint32_t base;

} __attribute__((packed)) gdt_ptr_t;

typedef struct tss_entry
{

    uint32_t previous;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;

} __attribute__((packed)) tss_entry_t;

extern void gdt_flush(gdt_ptr_t *pointer);
extern void tss_flush();

extern void gdt_set_gate(unsigned char index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
extern void gdt_init();

#endif

