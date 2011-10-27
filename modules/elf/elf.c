#include <lib/elf.h>
#include <lib/file.h>
#include <kernel/modules.h>
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

/*
char *elf_get_string_table(void *address, void *shaddress, unsigned int shsize, unsigned int shstringindex)
{

    return (char *)(address + elf_get_section_header_by_index(shaddress, shsize, shstringindex)->offset);

}
*/
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

void elf_print_symtab(void *address, struct elf_section_header *header, char *strtbl)
{

    unsigned int i;

    for (i = 0; i < header->size / header->esize; i++)
    {

        struct elf_symbol *symHeader = (struct elf_symbol *)(address + header->offset + i * header->esize);

//        struct elf_section_header *ih = elf_get_section_header(address, symHeader->shndx);

        unsigned int bind = symHeader->info >> 4;
        unsigned int type = symHeader->info & 0x0F;

        file_write_format(FILE_STDOUT, "[%d] Name: %s Val: 0x%x Sz: %d Info: 0x%x Bind: 0x%x Type: 0x%x Index: %d\n", i, strtbl + symHeader->name, symHeader->value, symHeader->size, symHeader->info, bind, type, symHeader->index);

    }

}

void elf_print_sections(void *shaddress, unsigned int shsize, unsigned int shcount, char *strtbl)
{

    unsigned int i;

    for (i = 0; i < shcount; i++)
    {

        struct elf_section_header *sHeader = elf_get_section_header_by_index(shaddress, shsize, i);

        file_write_format(FILE_STDOUT, "[%d] Name: %s Type: 0x%x Off: 0x%x Sz:0x%x\n", i, strtbl + sHeader->name, sHeader->type, sHeader->offset, sHeader->size);

    }

}

void doit()
{

//    file_write_format(FILE_STDOUT, "Module loaded\n");

}

static void elf_relocate_section(void *address, struct elf_section_header *header)
{

    int reloc = (int)address + (int)0x40;

    unsigned int i;

    for (i = 0; i < header->size / header->esize; i++)
    {

        struct elf_relocate *rHeader = (struct elf_relocate *)(address + header->offset + i * header->esize);

        unsigned int sym = rHeader->info >> 4;
        unsigned int type = rHeader->info & 0x0F;

        if (sym == 0x90)
        {


            int *s = (int *)(reloc + (int)rHeader->offset);

            int displacement = (int)0x00 - (int)*s;

            *s += (int)doit - reloc - (int)displacement - (int)rHeader->offset;

        }

        else
        {

        }

    }

}

void elf_relocate(void *address, struct elf_header *header)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_section_header *sHeader = elf_get_section_header_by_index(address + header->shoffset, header->shsize, i);

        if (sHeader->type == 0x09)
            elf_relocate_section(address, sHeader);

    }

}

void elf_init()
{

    elfModule.get_entry = elf_get_entry;
    elfModule.get_virtual = elf_get_virtual;

}

