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

static void elf_relocate_section(void *address, struct elf_header *header, struct elf_section_header *shHeader)
{

    struct elf_section_header *symHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 10);
    struct elf_section_header *strHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, 11);
    char *strtbl = (char *)(address + strHeader->offset);

    struct vfs_node *out = vfs_find("/stdout");

    int reloc = (int)address + (int)0x40;

    unsigned int i;

    for (i = 0; i < shHeader->size / shHeader->esize; i++)
    {

        struct elf_relocate *relocate = (struct elf_relocate *)(address + shHeader->offset + i * shHeader->esize);

        unsigned int sym = relocate->info >> 8;
        unsigned int type = relocate->info & 0x0F;

        struct elf_symbol *symbol = (struct elf_symbol *)(address + symHeader->offset + sym * symHeader->esize);
        char *name = strtbl + symbol->name;

        if (name && symbol->index == 0)
        {


            out->write(out, string_length(name), name);
            out->write(out, 1, "\n");

            char num[32];

            string_write_num(num, sym, 16);

            out->write(out, string_length(num), num);
            out->write(out, 1, "\n");

            int paddress = (int)kernel_get_symbol(name);

            int *s = (int *)(reloc + (int)relocate->offset);

            int displacement = (int)0x00 - (int)*s;

            *s += (int)paddress - reloc - (int)displacement - (int)relocate->offset;

        }

    }

}

void elf_relocate(void *address, struct elf_header *header)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *shHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, i);

        if (shHeader->type == 0x09)
            elf_relocate_section(address, header, shHeader);

    }

}

void elf_init()
{

    elfModule.get_entry = elf_get_entry;
    elfModule.get_virtual = elf_get_virtual;

}

