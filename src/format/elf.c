#include <fudge/memory.h>
#include "elf.h"

unsigned int elf_validate(struct elf_header *header)
{

    unsigned char id[] = {ELF_IDENTITY_MAGIC0, ELF_IDENTITY_MAGIC1, ELF_IDENTITY_MAGIC2, ELF_IDENTITY_MAGIC3};

    return memory_match(header->identify, id, 4);

}

unsigned int elf_findsymbol(struct elf_header *header, struct elf_sectionheader *sectionheader, struct elf_sectionheader *symbolheader, struct elf_symbol *symbols, char *strings, unsigned int count, char *symbol)
{

    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        char *s = strings + symbols[i].name;

        if (s[count] == '\0' && memory_match(symbol, s, count))
            return (header->type == ELF_TYPE_RELOCATABLE) ? sectionheader[symbols[i].shindex].address + sectionheader[symbols[i].shindex].offset + symbols[i].value : symbols[i].value;

    }

    return 0;

}

