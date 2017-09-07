#include <fudge.h>
#include <kernel.h>
#include "ioapic.h"

void ioapic_outd(unsigned int base, unsigned int index, unsigned int value)
{

    *(unsigned int *)(base) = index;
    *(unsigned int *)(base + 0x10) = value;

}

unsigned int ioapic_ind(unsigned int base, unsigned int index)
{

    *(unsigned int *)(base) = index;

    return *(unsigned int *)(base + 0x10);
}

void module_init(void)
{

}

