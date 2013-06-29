#include "../memory.h"
#include "elf.h"

unsigned int elf_validate(struct elf_header *header)
{

    char id[] = {ELF_IDENTITY_MAGIC0, ELF_IDENTITY_MAGIC1, ELF_IDENTITY_MAGIC2, ELF_IDENTITY_MAGIC3};

    return memory_match(header->identify, id, 4);

}

unsigned int elf_find_symbol(struct elf_header *header, struct elf_section_header *sectionheader, struct elf_section_header *symbolheader, struct elf_symbol *symbols, char *strings, unsigned int count, const char *symbol)
{

    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        if (memory_match(symbol, strings + symbols[i].name, count))
            return (header->type == ELF_TYPE_RELOCATABLE) ? sectionheader[symbols[i].shindex].address + sectionheader[symbols[i].shindex].offset + symbols[i].value : symbols[i].value;

    }

    return 0;

}

void elf_relocate_symbol(unsigned int address, unsigned int type, unsigned int addend)
{

    unsigned int *entry = (unsigned int *)(address);

    switch (type)
    {

        case ELF_RELOC_TYPE_32:

            *entry += addend;

            break;

        case ELF_RELOC_TYPE_PC32:

            *entry += addend - address;

            break;

    }

}

void elf_relocate_section(struct elf_section_header *sectionheader, struct elf_section_header *relocationheader, struct elf_section_header *dataheader, struct elf_relocation *relocations, struct elf_symbol *symbols, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        unsigned char type = relocations[i].info & 0x0F;
        unsigned char index = relocations[i].info >> 8;
        unsigned int offset = dataheader->offset + relocations[i].offset;
        unsigned int addend = sectionheader[symbols[index].shindex].offset + symbols[index].value;

        if (symbols[index].shindex)
            elf_relocate_symbol(address + offset, type, address + addend);
        else
            elf_relocate_symbol(address + offset, type, 0);

    }

}

