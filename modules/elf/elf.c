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

struct elf_program_header *elf_get_program_header(void *address)
{

    struct elf_header *header = elf_get_header(address);

    return (struct elf_program_header *)(address + header->phoffset);

}

struct elf_section_header *elf_get_section_header_by_index(void *address, unsigned int index)
{

    struct elf_header *header = elf_get_header(address);

    return (struct elf_section_header *)(address + header->shoffset + header->shsize * index);

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

    struct elf_program_header *pheader = elf_get_program_header(address);

    return pheader->vaddress;

}

void *elf_get_symbol(void *address, char *symname)
{

    struct elf_section_header *symHeader = elf_get_section_header_by_index(address, 11);
    struct elf_section_header *strHeader = elf_get_section_header_by_index(address, 12);
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

static void elf_relocate_section(void *address, struct elf_header *header, struct elf_section_header *relHeader)
{

    struct elf_section_header *txtHeader = elf_get_section_header_by_index(address, 1);
    struct elf_section_header *symHeader = elf_get_section_header_by_index(address, 11);
    struct elf_section_header *strHeader = elf_get_section_header_by_index(address, 12);
    char *strtbl = (char *)(address + strHeader->offset);

    unsigned int i;

    for (i = 0; i < relHeader->size / relHeader->esize; i++)
    {

        struct elf_relocate *relocate = (struct elf_relocate *)(address + relHeader->offset + i * relHeader->esize);

        unsigned int sym = relocate->info >> 8;
        unsigned int type = relocate->info & 0x0F;

        struct elf_symbol *symbol = (struct elf_symbol *)(address + symHeader->offset + sym * symHeader->esize);

        if (symbol->index)
        {

            int txtReloc = (int)address + txtHeader->offset;

            struct elf_section_header *ssymHeader = elf_get_section_header_by_index(address, symbol->index);

            int reloc = (int)address + ssymHeader->offset;
            int *entry = (int *)(txtReloc + relocate->offset);
            int paddress = reloc + relocate->offset;

            *entry = paddress - reloc + relocate->offset;

        }

        else
        {

            int txtReloc = (int)address + txtHeader->offset;
            int *entry = (int *)(txtReloc + relocate->offset);
            int paddress = (int)kernel_get_symbol(strtbl + symbol->name);

            if (!paddress)
                continue;

            *entry += *entry + paddress - txtReloc - relocate->offset;

        }

    }

}

void elf_relocate(void *address)
{

    void *offset = elf_get_symbol(address, "init");

    if (!offset)
        return;

    struct elf_header *header = elf_get_header(address);

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *relHeader = elf_get_section_header_by_index(address, i);

        if (i == 0x02)
            elf_relocate_section(address, header, relHeader);

    }

    header->entry = (void *)((unsigned int)address + 0x40 + (unsigned int)offset);

}

void elf_init()
{

    elfModule.get_entry = elf_get_entry;
    elfModule.get_virtual = elf_get_virtual;

}

