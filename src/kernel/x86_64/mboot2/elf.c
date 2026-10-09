#include <fudge.h>
#include <kernel.h>
#include <binary.h>
#include "elf.h"

static struct binary_format format;

static unsigned long findsymbol(unsigned long base, struct elf64_sectionheader *symbolheader, unsigned int count, char *symbolname)
{

    struct elf64_header *header = (struct elf64_header *)base;
    struct elf64_sectionheader *sectionheaders = (struct elf64_sectionheader *)(base + header->shoffset);
    struct elf64_symbol *symbols = (struct elf64_symbol *)(base + symbolheader->offset);
    char *strings = (char *)(base + sectionheaders[symbolheader->link].offset);

    if (symbolheader->size && symbolheader->esize)
    {

        unsigned int nsymbols = symbolheader->size / symbolheader->esize;
        unsigned int i;

        for (i = 0; i < nsymbols; i++)
        {

            if (!symbols[i].shindex)
                continue;

            if (strings[symbols[i].name + count] == '\0' && buffer_match(symbolname, &strings[symbols[i].name], count))
                return base + symbols[i].value + sectionheaders[symbols[i].shindex].address + sectionheaders[symbols[i].shindex].offset;

        }

    }

    return 0;

}

static unsigned int format_match(unsigned long base)
{

    struct elf64_header *header = (struct elf64_header *)base;

    return elf_validate((struct elf_header *)header) && header->identify[ELF_IDENTITY_CLASS] == ELF_IDENTITY_CLASS_64 && header->machine == ELF_MACHINE_X86_64;

}

static unsigned long format_findsymbol(unsigned long base, unsigned int count, char *symbolname)
{

    struct elf64_header *header = (struct elf64_header *)base;
    struct elf64_sectionheader *sectionheaders = (struct elf64_sectionheader *)(base + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        unsigned long address;

        if (sectionheaders[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        address = findsymbol(base, &sectionheaders[i], count, symbolname);

        if (address)
            return address;

    }

    return 0;

}

static unsigned long format_findentry(unsigned long base)
{

    struct elf64_header *header = (struct elf64_header *)base;

    return header->entry;

}

static unsigned int format_map(unsigned long base, unsigned long paddress, unsigned int size, struct mmap_header *mheader)
{

    struct elf64_header *header = (struct elf64_header *)base;
    struct elf64_programheader *programheaders = (struct elf64_programheader *)(base + header->phoffset);
    unsigned int used = 0;
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        struct elf64_programheader *programheader = &programheaders[i];
        unsigned int span = ((programheader->vaddress & 0xFFF) + programheader->msize + 0xFFF) & ~0xFFF;
        struct mmap_entry *entry;

        if (programheader->type != ELF_PROGRAM_TYPE_LOAD)
            continue;

        if (span > size - used)
            return 0;

        entry = mmap_allocate(mheader, MMAP_TYPE_BINARY, paddress + used, programheader->vaddress, programheader->msize, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);

        if (!entry)
            return 0;

        mmap_setbinary(entry, base + programheader->offset, programheader->fsize, programheader->msize);

        used += span;

    }

    return 1;

}

void elf_setup(void)
{

    binary_initformat(&format, format_match, format_findsymbol, format_findentry, format_map);
    resource_register(&format.resource);

}

