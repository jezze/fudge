#include <fudge.h>
#include <elf/elf.h>
#include "resource.h"
#include "task.h"
#include "service.h"
#include "binary.h"

static struct binary_format format;

static unsigned int relocate(struct binary_node *node, struct elf_sectionheader *relocationheader, unsigned int address)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    struct elf_sectionheader *dataheader = (struct elf_sectionheader *)(node->physical + header->shoffset + relocationheader->info * header->shsize);
    struct elf_sectionheader *symbolheader = (struct elf_sectionheader *)(node->physical + header->shoffset + relocationheader->link * header->shsize);
    unsigned int count = relocationheader->size / relocationheader->esize;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        struct elf_relocation *relocation = (struct elf_relocation *)(node->physical + relocationheader->offset + i * relocationheader->esize);
        struct elf_symbol *symbol = (struct elf_symbol *)(node->physical + symbolheader->offset + (relocation->info >> 8) * symbolheader->esize);
        unsigned long *entry = (unsigned long *)(address + dataheader->offset + relocation->offset);
        unsigned int addend = 0;

        if (symbol->shindex)
        {

            struct elf_sectionheader *referenceheader = (struct elf_sectionheader *)(node->physical + header->shoffset + symbol->shindex * header->shsize);

            addend = address + referenceheader->offset + symbol->value;

        }

        switch (relocation->info & 0x0F)
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
    struct elf_sectionheader *stringheader = (struct elf_sectionheader *)(node->physical + header->shoffset + symbolheader->link * header->shsize);
    char *strings = (char *)(node->physical + stringheader->offset);
    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        struct elf_symbol *symbol = (struct elf_symbol *)(node->physical + symbolheader->offset + i * symbolheader->esize);

        if (!symbol->shindex)
            continue;

        if (strings[symbol->name + count] == '\0' && memory_match(symbolname, strings + symbol->name, count))
        {

            struct elf_sectionheader *referenceheader = (struct elf_sectionheader *)(node->physical + header->shoffset + symbol->shindex * header->shsize);

            return symbol->value + referenceheader->address + referenceheader->offset;

        }

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
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_sectionheader *referenceheader = (struct elf_sectionheader *)(node->physical + header->shoffset + i * header->shsize);
        unsigned int address;

        if (referenceheader->type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        address = findsymbol(node, referenceheader, count, symbolname);

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
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        struct elf_programheader *programheader = (struct elf_programheader *)(node->physical + header->phoffset + i * header->phsize);

        if (programheader->vaddress <= address && programheader->vaddress + programheader->msize > address)
            return programheader->vaddress;

    }

    return 0;

}

static unsigned int format_copyprogram(struct binary_node *node)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    unsigned int i;

    for (i = 0; i < header->phcount; i++)
    {

        struct elf_programheader *programheader = (struct elf_programheader *)(node->physical + header->phoffset + i * header->phsize);

        if (programheader->fsize)
            memory_copy((void *)programheader->vaddress, (void *)(node->physical + programheader->offset), programheader->fsize);

        memory_clear((void *)(programheader->vaddress + programheader->fsize), programheader->msize - programheader->fsize);

    }

    return 1;

}

static unsigned int format_relocate(struct binary_node *node, unsigned int address)
{

    struct elf_header *header = (struct elf_header *)node->physical;
    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_sectionheader *referenceheader = (struct elf_sectionheader *)(node->physical + header->shoffset + i * header->shsize);

        referenceheader->address += address;

        if (referenceheader->type != ELF_SECTION_TYPE_REL)
            continue;

        if (!relocate(node, referenceheader, address))
            return 0;

    }

    return 1;

}

void binary_setupelf(void)
{

    binary_initformat(&format, format_match, format_findsymbol, format_findentry, format_findbase, format_copyprogram, format_relocate);
    resource_register(&format.resource);

}

