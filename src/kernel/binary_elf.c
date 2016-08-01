#include <fudge.h>
#include <format/elf.h>
#include "resource.h"
#include "binary.h"

static struct binary_format format;

static unsigned int relocate(struct binary_node *node, struct elf_sectionheader *relocationheader)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(node->physical + header->shoffset);
    struct elf_relocation *relocations = (struct elf_relocation *)(node->physical + relocationheader->offset);
    struct elf_symbol *symbols = (struct elf_symbol *)(node->physical + sectionheaders[relocationheader->link].offset);
    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        struct elf_symbol *symbol = &symbols[(relocations[i].info >> 8)];
        unsigned long *entry = (unsigned long *)(node->physical + sectionheaders[relocationheader->info].offset + relocations[i].offset);
        unsigned int addend = (symbol->shindex) ? node->physical + sectionheaders[symbol->shindex].offset + symbol->value : 0;

        switch (relocations[i].info & 0x0F)
        {

        case ELF_RELOC_TYPE_32:
            *entry += addend;

            break;

        case ELF_RELOC_TYPE_PC32:
            *entry += addend - (unsigned long)entry;

            break;

        }

    }

    return 1;

}

static unsigned long findsymbol(struct binary_node *node, struct elf_sectionheader *symbolheader, unsigned int count, char *symbolname)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(node->physical + header->shoffset);
    struct elf_symbol *symbols = (struct elf_symbol *)(node->physical + symbolheader->offset);
    char *strings = (char *)(node->physical + sectionheaders[symbolheader->link].offset);
    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        if (!symbols[i].shindex)
            continue;

        if (strings[symbols[i].name + count] == '\0' && memory_match(symbolname, &strings[symbols[i].name], count))
            return symbols[i].value + sectionheaders[symbols[i].shindex].address + sectionheaders[symbols[i].shindex].offset;

    }

    return 0;

}

static unsigned int format_match(struct binary_node *node)
{

    struct elf_header *header = (struct elf_header *)node->physical;

    return elf_validate(header);

}

static unsigned long format_findsymbol(struct binary_node *node, unsigned int count, char *symbolname)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(node->physical + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        unsigned int address;

        if (sectionheaders[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        address = findsymbol(node, &sectionheaders[i], count, symbolname);

        if (address)
            return address;

    }

    return 0;

}

static unsigned long format_findentry(struct binary_node *node)
{

    struct elf_header *header = (struct elf_header *)node->physical;

    return header->entry;

}

static unsigned long format_findbase(struct binary_node *node, unsigned long address)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_programheader *programheaders = (struct elf_programheader *)(node->physical + header->phoffset);
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        if (programheaders[i].vaddress <= address && programheaders[i].vaddress + programheaders[i].msize > address)
            return programheaders[i].vaddress;

    }

    return 0;

}

static unsigned int format_copyprogram(struct binary_node *node)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_programheader *programheaders = (struct elf_programheader *)(node->physical + header->phoffset);
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        if (programheaders[i].fsize)
            memory_copy((void *)programheaders[i].vaddress, (void *)(node->physical + programheaders[i].offset), programheaders[i].fsize);

        memory_clear((void *)(programheaders[i].vaddress + programheaders[i].fsize), programheaders[i].msize - programheaders[i].fsize);

    }

    return 1;

}

static unsigned int format_relocate(struct binary_node *node)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_sectionheader *sectionheaders = (struct elf_sectionheader *)(node->physical + header->shoffset);
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        sectionheaders[i].address += node->physical;

        if (sectionheaders[i].type != ELF_SECTION_TYPE_REL)
            continue;

        if (!relocate(node, &sectionheaders[i]))
            return 0;

    }

    return 1;

}

void binary_setupelf(void)
{

    binary_initformat(&format, format_match, format_findsymbol, format_findentry, format_findbase, format_copyprogram, format_relocate);
    resource_register(&format.resource);

}

