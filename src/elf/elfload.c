#include <abi.h>
#include <fudge.h>
#include "elf.h"

static unsigned int readheader(unsigned int id, struct elf_header *header)
{

    return file_seekreadall(id, header, ELF_HEADER_SIZE, 0);

}

static unsigned int readsectionheader(unsigned int id, struct elf_header *header, unsigned int index, struct elf_sectionheader *sectionheader)
{

    return file_seekreadall(id, sectionheader, header->shsize, header->shoffset + index * header->shsize);

}

static unsigned int findsymbol(unsigned int id, unsigned int count, char *symbolname)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(id, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader symbolheader;
        struct elf_sectionheader stringheader;
        char strings[FUDGE_BSIZE];
        unsigned int j;

        if (!readsectionheader(id, &header, i, &symbolheader))
            return 0;

        if (symbolheader.type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        if (!readsectionheader(id, &header, symbolheader.link, &stringheader))
            return 0;

        if (stringheader.size > FUDGE_BSIZE)
            return 0;

        if (!file_seekreadall(id, strings, stringheader.size, stringheader.offset))
            return 0;

        for (j = 0; j < symbolheader.size / symbolheader.esize; j++)
        {

            struct elf_symbol symbol;

            if (!file_seekreadall(id, &symbol, symbolheader.esize, symbolheader.offset + j * symbolheader.esize))
                return 0;

            if (strings[symbol.name + count] == '\0' && memory_match(symbolname, &strings[symbol.name], count))
            {

                if (header.type == ELF_TYPE_RELOCATABLE)
                {

                    struct elf_sectionheader referenceheader;

                    if (!readsectionheader(id, &header, symbol.shindex, &referenceheader))
                        return 0;

                    return referenceheader.address + referenceheader.offset + symbol.value;

                }

                else
                {

                    return symbol.value;

                }

            }

        }

    }

    return 0;

}

static unsigned int findkernelsymbol(unsigned int count, char *symbolname)
{

    unsigned int address;

    file_open(CALL_L0);

    address = findsymbol(CALL_L0, count, symbolname);

    file_close(CALL_L0);

    return address;

}

static unsigned int findmodulesymbol(unsigned int count, char *symbolname)
{

    unsigned int length = memory_findbyte(symbolname, count, '_') - 1;
    unsigned int offset = 0;
    unsigned int address;
    char module[32];

    offset += memory_write(module, 32, symbolname, length, offset);
    offset += memory_write(module, 32, ".ko", 4, offset);

    if (!file_walkfrom(CALL_L2, CALL_L1, module))
        return 0;

    file_open(CALL_L2);

    address = findsymbol(CALL_L2, count, symbolname);

    file_close(CALL_L2);

    return address;

}

static unsigned int resolvesymbols(unsigned int id, struct elf_sectionheader *relocationheader, struct elf_sectionheader *symbolheader, char *strings, unsigned int offset)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        unsigned char index;
        unsigned int address;
        unsigned int value;
        char *symbolname;
        unsigned int count;
        struct elf_relocation relocation;
        struct elf_symbol symbol;

        if (!file_seekreadall(id, &relocation, relocationheader->esize, relocationheader->offset + i * relocationheader->esize))
            return 0;

        index = relocation.info >> 8;

        if (!file_seekreadall(id, &symbol, symbolheader->esize, symbolheader->offset + index * symbolheader->esize))
            return 0;

        if (symbol.shindex)
            continue;

        symbolname = strings + symbol.name;
        count = ascii_length(symbolname);
        address = findmodulesymbol(count, symbolname);

        if (!address)
            address = findkernelsymbol(count, symbolname);

        if (!address)
            return 0;

        if (!file_seekreadall(id, &value, 4, offset + relocation.offset))
            return 0;

        value += address;

        if (!file_seekwriteall(id, &value, 4, offset + relocation.offset))
            return 0;

    }

    return 1;

}

static unsigned int resolve(unsigned int id)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(id, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader relocationheader;
        struct elf_sectionheader dataheader;
        struct elf_sectionheader symbolheader;
        struct elf_sectionheader stringheader;
        char strings[FUDGE_BSIZE];

        if (!readsectionheader(id, &header, i, &relocationheader))
            return 0;

        if (relocationheader.type != ELF_SECTION_TYPE_REL)
            continue;

        if (!readsectionheader(id, &header, relocationheader.info, &dataheader))
            return 0;

        if (!readsectionheader(id, &header, relocationheader.link, &symbolheader))
            return 0;

        if (!readsectionheader(id, &header, symbolheader.link, &stringheader))
            return 0;

        if (stringheader.size > FUDGE_BSIZE)
            return 0;

        if (!file_seekreadall(id, strings, stringheader.size, stringheader.offset))
            return 0;

        if (!resolvesymbols(id, &relocationheader, &symbolheader, strings, dataheader.offset))
            return 0;

    }

    return 1;

}

void main(void)
{

    if (!file_walk(CALL_L0, "/bin/fudge"))
        return;

    if (!file_walk(CALL_L1, "/mod/"))
        return;

    file_open(CALL_PI);

    if (resolve(CALL_PI))
        call_load(CALL_PI);

    file_close(CALL_PI);

}

