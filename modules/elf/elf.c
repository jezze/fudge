#include <lib/elf.h>
#include <lib/file.h>
#include <lib/string.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/elf/elf.h>

static struct elf_module elfModule;

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

struct elf_program_header *elf_get_program_header(void *phaddress)
{

    return (struct elf_program_header *)(phaddress);

}

struct elf_section_header *elf_get_section_header_by_index(void *shaddress, unsigned int shsize, unsigned int index)
{

    return (struct elf_section_header *)(shaddress + shsize * index);

}

struct elf_section_header *elf_get_section_header_by_type(void *shaddress, unsigned int shsize, unsigned int shcount, unsigned int type, unsigned int flags)
{

    unsigned int i;

    for (i = 0; i < shcount; i++)
    {

        struct elf_section_header *sHeader = elf_get_section_header_by_index(shaddress, shsize, i);

        if (sHeader->type == type)
        {

            if (!flags)
                return sHeader;

            if (sHeader->flags & flags)
                return sHeader;

        }

    }

    return 0;

}

void *elf_get_entry(void *address)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return 0;

    return header->entry;

}

void *elf_get_virtual(void *address)
{

    struct elf_header *header = elf_get_header(address);

    if (!header)
        return 0;

    struct elf_program_header *pheader = elf_get_program_header(address + header->phoffset);

    return pheader->vaddress;

}

void *elf_get_symbol(void *address, struct elf_header *header, char *symname)
{

    struct elf_section_header *symHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 11);
    struct elf_section_header *strHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 12);
    char *strtbl = (char *)(address + strHeader->offset);

    unsigned int i;

    for (i = 0; i < symHeader->size / symHeader->esize; i++)
    {

        struct elf_symbol *symbol = (struct elf_symbol *)(address + symHeader->offset + i * symHeader->esize);

        if (!symbol->name)
            continue;

        char *name = strtbl + symbol->name;

        if (!string_compare(symname, name))
            return (void *)symbol->value;

    }

    return 0;

}

static void elf_relocate_section(void *address, struct elf_header *header, struct elf_section_header *shHeader)
{

    struct elf_section_header *symHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 11);
    struct elf_section_header *strHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 12);
    char *strtbl = (char *)(address + strHeader->offset);

    unsigned int i;

    for (i = 0; i < shHeader->size / shHeader->esize; i++)
    {

        struct elf_relocate *relocate = (struct elf_relocate *)(address + shHeader->offset + i * shHeader->esize);

        unsigned int sym = relocate->info >> 8;
        unsigned int type = relocate->info & 0x0F;

        struct elf_symbol *symbol = (struct elf_symbol *)(address + symHeader->offset + sym * symHeader->esize);

        int symReloc = (int)address + (int)0x40;

        int *entry = (int *)(symReloc + relocate->offset);

        if (symbol->index)
        {

            struct elf_section_header *ssymHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, symbol->index);

            int reloc = (int)address + ssymHeader->offset;

            if (symbol->index == 7)
            {

                int paddress = reloc + relocate->offset;
                *entry = paddress - reloc + relocate->offset;

            }

            else
            {

                int paddress = reloc + relocate->offset;
                *entry = paddress - reloc + relocate->offset;

            }

        }

        else
        {

            int paddress = (int)kernel_get_symbol(strtbl + symbol->name);

            if (!paddress)
                continue;

            *entry += *entry + paddress - symReloc - relocate->offset;

        }

    }

}

void elf_relocate(void *address, struct elf_header *header)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *shHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, i);

        if (i == 0x02)
            elf_relocate_section(address, header, shHeader);

    }

}

void elf_init()
{

    elfModule.get_entry = elf_get_entry;
    elfModule.get_virtual = elf_get_virtual;

}

