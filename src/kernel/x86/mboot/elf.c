#include <fudge.h>
#include <kernel.h>
#include "elf.h"

static struct binary_format format;

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

static unsigned int format_readsection(unsigned int base, struct mmap_entry *entry, unsigned int index)
{

    struct elf_header *header = (struct elf_header *)base;
    struct elf_programheader *programheaders = (struct elf_programheader *)(base + header->phoffset);

    if (index < header->phcount)
    {

        struct elf_programheader *programheader = &programheaders[index];

        mmap_initentry(entry, base + programheader->offset, programheader->fsize, programheader->msize, programheader->flags, programheader->vaddress);

        return index + 1;

    }

    return 0;

}

void elf_setup(void)
{

    binary_initformat(&format, format_match, format_findsymbol, format_findentry, format_readsection);
    resource_register(&format.resource);

}

