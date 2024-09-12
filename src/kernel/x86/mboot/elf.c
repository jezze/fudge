#include <fudge.h>
#include <kernel.h>
#include "elf.h"

static struct binary_format format;

static unsigned int relocate(unsigned int base, struct elf_sectionheader *relocationheader)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(base + header->shoffset);
    struct elf_relocation *relocations = (struct elf_relocation *)(base + relocationheader->offset);
    struct elf_symbol *symbols = (struct elf_symbol *)(base + sectionheaders[relocationheader->link].offset);
    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        struct elf_symbol *symbol = &symbols[(relocations[i].info >> 8)];
        unsigned int *entry = (unsigned int *)(base + sectionheaders[relocationheader->info].offset + relocations[i].offset);
        unsigned int addend = (symbol->shindex) ? base + sectionheaders[symbol->shindex].offset + symbol->value : 0;

        switch (relocations[i].info & 0x0F)
        {

        case ELF_RELOC_TYPE_32:
            *entry += addend;

            break;

        case ELF_RELOC_TYPE_PC32:
            *entry += addend - (unsigned int)entry;

            break;

        }

    }

    return 1;

}

static unsigned int findsymbol(unsigned int base, struct elf_sectionheader *symbolheader, unsigned int count, char *symbolname)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(base + header->shoffset);
    struct elf_symbol *symbols = (struct elf_symbol *)(base + symbolheader->offset);
    char *strings = (char *)(base + sectionheaders[symbolheader->link].offset);
    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        if (!symbols[i].shindex)
            continue;

        if (strings[symbols[i].name + count] == '\0' && buffer_match(symbolname, &strings[symbols[i].name], count))
            return base + symbols[i].value + sectionheaders[symbols[i].shindex].address + sectionheaders[symbols[i].shindex].offset;

    }

    return 0;

}

static unsigned int format_match(unsigned int base)
{

    struct elf_header *header = (struct elf_header *)base;

    return elf_validate(header);

}

static unsigned int format_findsymbol(unsigned int base, unsigned int count, char *symbolname)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(base + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        unsigned int address;

        if (sectionheaders[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        address = findsymbol(base, &sectionheaders[i], count, symbolname);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int format_findentry(unsigned int base)
{

    struct elf_header *header = (struct elf_header *)base;

    return header->entry;

}

static unsigned int format_findbase(unsigned int base, unsigned int address)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_programheader *programheaders = (struct elf_programheader *)(base + header->phoffset);
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        if (programheaders[i].vaddress <= address && programheaders[i].vaddress + programheaders[i].msize > address)
            return programheaders[i].vaddress;

    }

    return 0;

}

static unsigned int format_copyprogram(unsigned int base)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_programheader *programheaders = (struct elf_programheader *)(base + header->phoffset);
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        if (programheaders[i].fsize)
            buffer_copy((void *)programheaders[i].vaddress, (void *)(base + programheaders[i].offset), programheaders[i].fsize);

        buffer_clear((void *)(programheaders[i].vaddress + programheaders[i].fsize), programheaders[i].msize - programheaders[i].fsize);

    }

    return 1;

}

static void format_relocate(unsigned int base)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(base + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_REL)
            relocate(base, &sectionheaders[i]);

    }

}

void elf_setup(void)
{

    binary_initformat(&format, format_match, format_findsymbol, format_findentry, format_findbase, format_copyprogram, format_relocate);
    resource_register(&format.resource);

}

