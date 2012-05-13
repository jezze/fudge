#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>

struct elf_header *elf_get_header(void *address)
{

    struct elf_header *header = (struct elf_header *)address;

    if (header->identify[0] != ELF_IDENTITY_MAGIC0)
        return 0;

    if (header->identify[1] != ELF_IDENTITY_MAGIC1)
        return 0;

    if (header->identify[2] != ELF_IDENTITY_MAGIC2)
        return 0;

    if (header->identify[3] != ELF_IDENTITY_MAGIC3)
        return 0;

    return header;

}

unsigned int elf_get_virtual(void *address)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return 0;

    struct elf_program_header *pheader = (struct elf_program_header *)((unsigned int)address + header->phoffset);

    return pheader->vaddress;

}

unsigned int elf_get_symbol(void *address, char *name)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return 0;

    struct elf_section_header *sheader = (struct elf_section_header *)((unsigned int)address + header->shoffset);
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *symHeader = &sheader[relHeader->link];

    struct elf_symbol *symTable = (struct elf_symbol *)((unsigned int)address + symHeader->offset);
    char *infoTable = (char *)address + sheader[relHeader->info].offset;
    char *strTable = (char *)address + sheader[symHeader->link].offset;

    unsigned int i;
    unsigned int count = symHeader->size / symHeader->esize;

    for (i = 0; i < count; i++)
    {

        struct elf_symbol *symEntry = &symTable[i];

        if (memory_compare(name, strTable + symEntry->name, string_length(name)))
            return (unsigned int)(infoTable + symEntry->value);

    }

    return 0;

}

void elf_prepare(void *address)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return;

    struct elf_section_header *sheader = (struct elf_section_header *)((unsigned int)address + header->shoffset);

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sheader[i].type == 8)
            memory_clear((char *)address + sheader[i].offset, sheader[i].size);

    }

}

void elf_relocate(void *address)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return;

    struct elf_section_header *sheader = (struct elf_section_header *)((unsigned int)address + header->shoffset);
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *symHeader = &sheader[relHeader->link];

    struct elf_relocate *relTable = (struct elf_relocate *)((unsigned int)address + relHeader->offset);
    struct elf_symbol *symTable = (struct elf_symbol *)((unsigned int)address + symHeader->offset);
    char *infoTable = (char *)address + sheader[relHeader->info].offset;

    unsigned int i;
    unsigned int count = relHeader->size / relHeader->esize; 

    for (i = 0; i < count; i++)
    {

        struct elf_relocate *relEntry = &relTable[i];

        unsigned char type = relEntry->info & 0x0F;
        unsigned char index = relEntry->info >> 8;

        struct elf_symbol *symEntry = &symTable[index];
        unsigned int *entry = (unsigned int *)(infoTable + relEntry->offset);
        unsigned int value = *entry;

        unsigned int addend = (symEntry->shindex) ? (unsigned int)address + sheader[symEntry->shindex].offset + symEntry->value : 0;

        switch (type)
        {

            case 1:

                *entry = value + addend;

                break;

            case 2:

                *entry = value + addend - (unsigned int)entry;

                break;

        }

    }

}

void elf_symbolize(void *address, unsigned int (*get_symbol)(char *name))
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return;

    struct elf_section_header *sheader = (struct elf_section_header *)((unsigned int)address + header->shoffset);
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *symHeader = &sheader[relHeader->link];

    struct elf_relocate *relTable = (struct elf_relocate *)((unsigned int)address + relHeader->offset);
    struct elf_symbol *symTable = (struct elf_symbol *)((unsigned int)address + symHeader->offset);
    char *infoTable = (char *)address + sheader[relHeader->info].offset;
    char *strTable = (char *)address + sheader[symHeader->link].offset;

    unsigned int i;
    unsigned int count = relHeader->size / relHeader->esize; 

    for (i = 0; i < count; i++)
    {

        struct elf_relocate *relEntry = &relTable[i];

        unsigned char type = relEntry->info & 0x0F;
        unsigned char index = relEntry->info >> 8;

        struct elf_symbol *symEntry = &symTable[index];
        unsigned int *entry = (unsigned int *)(infoTable + relEntry->offset);
        unsigned int value = *entry;

        if (symEntry->shindex)
            continue;

        unsigned int addend = get_symbol(strTable + symEntry->name);

        switch (type)
        {

            case 1:

                *entry = value + addend;

                break;

            case 2:

                *entry = value + addend;

                break;

        }

    }

}

