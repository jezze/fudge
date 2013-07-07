#include <fudge/kernel.h>
#include "idt.h"

#define IDT_ENTRIES                     256

static struct idt_entry entries[IDT_ENTRIES];
static struct idt_pointer pointer;

void idt_set_entry(struct idt_pointer *p, enum idt_index index, void (*callback)(), unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    p->base[index].baseLow = (base & 0xFFFF);
    p->base[index].baseHigh = (base >> 16);
    p->base[index].selector = selector;
    p->base[index].flags = flags;

}

struct idt_pointer *idt_setup_pointer()
{

    pointer.base = entries;
    pointer.limit = (sizeof (struct idt_entry) * IDT_ENTRIES) - 1;

    return &pointer;

}

