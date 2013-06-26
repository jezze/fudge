#include <fudge/kernel.h>
#include "idt.h"

static struct idt_entry entries[IDT_ENTRY_SLOTS];
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
    pointer.limit = (sizeof (struct idt_entry) * IDT_ENTRY_SLOTS) - 1;

    return &pointer;

}

