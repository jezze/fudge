#include <lib/elf.h>
#include <lib/string.h>
#include <kernel/elf.h>
#include <kernel/symbol.h>

static struct elf_header *get_header(void *address)
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

void *elf_get_entry(void *address)
{

    struct elf_header *header = get_header(address);

    if (!header)
        return 0;

    return header->entry;

}

void *elf_get_virtual(void *address)
{

    struct elf_header *header = get_header(address);

    if (!header)
        return 0;

    struct elf_program_header *pheader = address + header->phoffset;

    return pheader->vaddress;

}

void *elf_get_symbol(void *address, char *name)
{

    struct elf_header *header = get_header(address);

    if (!header)
        return 0;

    struct elf_section_header *sheader = address + header->shoffset;
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *infoHeader = &sheader[relHeader->info];
    struct elf_section_header *symHeader = &sheader[relHeader->link];
    struct elf_section_header *strHeader = &sheader[symHeader->link];

    struct elf_symbol *symTable = address + symHeader->offset;
    void *infoTable = address + infoHeader->offset;
    char *strTable = address + strHeader->offset;

    unsigned int i;
    unsigned int count = symHeader->size / symHeader->esize;

    for (i = 0; i < count; i++)
    {

        struct elf_symbol *symbol = &symTable[i];

        if (!string_compare(name, strTable + symbol->name))
            return infoTable + symbol->value;

    }

    return 0;

}

void elf_relocate(void *address)
{

    struct elf_header *header = get_header(address);

    if (!header)
        return;

    struct elf_section_header *sheader = address + header->shoffset;
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *infoHeader = &sheader[relHeader->info];
    struct elf_section_header *symHeader = &sheader[relHeader->link];
    struct elf_section_header *strHeader = &sheader[symHeader->link];

    struct elf_relocate *relTable = address + relHeader->offset;
    struct elf_symbol *symTable = address + symHeader->offset;
    void *infoTable = address + infoHeader->offset;
    char *strTable = address + strHeader->offset;

    unsigned int i;
    unsigned int count = relHeader->size / relHeader->esize; 

    for (i = 0; i < count; i++)
    {

        struct elf_relocate *relocate = &relTable[i];

        unsigned char type = relocate->info & 0x0F;
        unsigned char index = relocate->info >> 8;

        struct elf_symbol *symbol = &symTable[index];
        unsigned int *entry = (unsigned int *)(infoTable + relocate->offset);
        unsigned int value = *entry;
        unsigned int addend = (symbol->shindex) ? (unsigned int)(address + sheader[symbol->shindex].offset + symbol->value) : (unsigned int)symbol_find(strTable + symbol->name);

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

