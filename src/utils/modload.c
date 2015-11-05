#include <abi.h>
#include <fudge.h>
#include <format/elf.h>

static unsigned int readheader(unsigned int id, struct elf_header *header)
{

    call_seek(id, 0);

    return call_read(id, ELF_HEADER_SIZE, header);

}

static unsigned int readsectionheader(unsigned int id, struct elf_header *header, unsigned int index, struct elf_sectionheader *sectionheader)
{

    call_seek(id, header->shoffset + index * header->shsize);

    return call_read(id, header->shsize, sectionheader);

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

        call_seek(id, stringheader.offset);

        if (!call_read(id, stringheader.size, strings))
            return 0;

        for (j = 0; j < symbolheader.size / symbolheader.esize; j++)
        {

            struct elf_symbol symbol;
            char *s;

            call_seek(id, symbolheader.offset + j * symbolheader.esize);

            if (!call_read(id, symbolheader.esize, &symbol))
                return 0;

            s = strings + symbol.name;

            if (s[count] == '\0' && memory_match(symbolname, s, count))
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

    call_open(CALL_L0);

    address = findsymbol(CALL_L0, count, symbolname);

    call_close(CALL_L0);

    return address;

}

static unsigned int findmodulesymbol(unsigned int count, char *symbolname)
{

    unsigned int length = memory_findbyte(symbolname, count, '_') - 1;
    unsigned int offset = 0;
    unsigned int address;
    char module[32];

    offset += memory_write(module, 32, symbolname, length, offset);
    offset += memory_write(module, 32, ".ko", 3, offset);

    if (!call_walk(CALL_L2, CALL_L1, offset, module))
        return 0;

    call_open(CALL_L2);

    address = findsymbol(CALL_L2, count, symbolname);

    call_close(CALL_L2);

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

        call_seek(id, relocationheader->offset + i * relocationheader->esize);

        if (!call_read(id, relocationheader->esize, &relocation))
            return 0;

        index = relocation.info >> 8;

        call_seek(id, symbolheader->offset + index * symbolheader->esize);

        if (!call_read(id, symbolheader->esize, &symbol))
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

        call_seek(id, offset + relocation.offset);

        if (!call_read(id, 4, &value))
            return 0;

        value += address;

        call_seek(id, offset + relocation.offset);

        if (!call_write(id, 4, &value))
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

        call_seek(id, stringheader.offset);

        if (!call_read(id, stringheader.size, strings))
            return 0;

        if (!resolvesymbols(id, &relocationheader, &symbolheader, strings, dataheader.offset))
            return 0;

    }

    return 1;

}

void main(void)
{

    if (!call_walk(CALL_L0, CALL_PR, 9, "bin/fudge"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 4, "mod/"))
        return;

    call_open(CALL_PI);

    if (resolve(CALL_PI))
        call_load(CALL_PI);

    call_close(CALL_PI);

}

