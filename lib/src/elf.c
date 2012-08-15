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

