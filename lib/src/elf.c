#include <elf.h>
#include <memory.h>
#include <string.h>

unsigned int elf_validate(struct elf_header *header)
{

    char id[] = {ELF_IDENTITY_MAGIC0, ELF_IDENTITY_MAGIC1, ELF_IDENTITY_MAGIC2, ELF_IDENTITY_MAGIC3};

    return memory_match(header->identify, id, 4);

}

unsigned int elf_find_section(struct elf_header *header, struct elf_section_header *sectionHeader, unsigned int type)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionHeader[i].type == type)
            return i;

    }

    return 0;

}

unsigned int elf_find_symbol(struct elf_header *header, struct elf_section_header *sectionHeader, unsigned int symbolHeaderIndex, struct elf_symbol *symbolTable, char *stringTable, char *symbol)
{

    unsigned int i;

    for (i = 0; i < sectionHeader[symbolHeaderIndex].size / sectionHeader[symbolHeaderIndex].esize; i++)
    {

        if (!memory_match(symbol, stringTable + symbolTable[i].name, string_length(symbol)))
            continue;

        return (header->type == ELF_TYPE_RELOCATABLE) ? sectionHeader[symbolTable[i].shindex].address + sectionHeader[symbolTable[i].shindex].offset + symbolTable[i].value : symbolTable[i].value;

    }

    return 0;

}

void elf_relocate_symbol(unsigned int address, unsigned int offset, unsigned int type, unsigned int addend)
{

    unsigned int *entry = (unsigned int *)(address + offset);

    switch (type)
    {

        case 1:

            *entry += addend;

            break;

        case 2:

            *entry += addend - offset - address;

            break;

    }

}

void elf_relocate_section(struct elf_section_header *sectionHeader, unsigned int relocateHeaderIndex, unsigned int relocateDataIndex, struct elf_relocate *relocateTable, struct elf_symbol *symbolTable, unsigned int address)
{

    unsigned int i;

    for (i = 0; i < sectionHeader[relocateHeaderIndex].size / sectionHeader[relocateHeaderIndex].esize; i++)
    {

        unsigned char type = relocateTable[i].info & 0x0F;
        unsigned char index = relocateTable[i].info >> 8;
        struct elf_symbol *symbol = &symbolTable[index];
        unsigned int offset = sectionHeader[relocateDataIndex].offset + relocateTable[i].offset;

        if (symbol->shindex)
            elf_relocate_symbol(address, offset, type, address + sectionHeader[symbol->shindex].offset + symbol->value);
        else
            elf_relocate_symbol(address, offset, type, 0);

    }

}

