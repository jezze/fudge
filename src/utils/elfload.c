#include <fudge.h>
#include <abi.h>

static unsigned int readheader(unsigned int descriptor, struct elf_header *header)
{

    return call_read_all(descriptor, header, ELF_HEADER_SIZE, 0);

}

static unsigned int readsectionheader(unsigned int descriptor, struct elf_header *header, unsigned int index, struct elf_sectionheader *sectionheader)
{

    return call_read_all(descriptor, sectionheader, header->shsize, header->shoffset + index * header->shsize);

}

static unsigned int findvalue(unsigned int descriptor, struct elf_header *header, struct elf_sectionheader *symbolheader, char *strings, unsigned int count, char *symbolname)
{

    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        struct elf_symbol symbol;

        if (!call_read_all(descriptor, &symbol, symbolheader->esize, symbolheader->offset + i * symbolheader->esize))
            return 0;

        if (strings[symbol.name + count] != '\0')
            continue;

        if (!buffer_match(symbolname, &strings[symbol.name], count))
            continue;

        if (header->type == ELF_TYPE_RELOCATABLE)
        {

            struct elf_sectionheader referenceheader;

            if (!readsectionheader(descriptor, header, symbol.shindex, &referenceheader))
                return 0;

            return referenceheader.address + referenceheader.offset + symbol.value;

        }

        return symbol.value;

    }

    return 0;

}

static unsigned int findsymbol(unsigned int descriptor, unsigned int count, char *symbolname)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(descriptor, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader symbolheader;
        struct elf_sectionheader stringheader;
        char strings[BUFFER_SIZE * 4];
        unsigned int value;

        if (!readsectionheader(descriptor, &header, i, &symbolheader))
            return 0;

        if (symbolheader.type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        if (!readsectionheader(descriptor, &header, symbolheader.link, &stringheader))
            return 0;

        if (stringheader.size > BUFFER_SIZE * 4)
            return 0;

        if (!call_read_all(descriptor, strings, stringheader.size, stringheader.offset))
            return 0;

        value = findvalue(descriptor, &header, &symbolheader, strings, count, symbolname);

        if (value)
            return value;

    }

    return 0;

}

static unsigned int findmodulesymbol(unsigned int count, char *symbolname)
{

    unsigned int address = 0;
    unsigned int underscore = buffer_findbyte(symbolname, count, '_');
    char module[32];

    cstring_write_fmt2(module, 32, "%w.ko\\0", 0, symbolname, &underscore);

    if (call_walk_relative(FILE_L0, FILE_G0, module))
        address = findsymbol(FILE_L0, count, symbolname);

    if (!address)
        address = findsymbol(FILE_G1, count, symbolname);

    return address;

}

static unsigned int resolvesymbols(unsigned int descriptor, struct elf_sectionheader *relocationheader, struct elf_sectionheader *symbolheader, char *strings, unsigned int offset)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        unsigned char index;
        unsigned int address;
        struct elf_relocation relocation;
        struct elf_symbol symbol;

        if (!call_read_all(descriptor, &relocation, relocationheader->esize, relocationheader->offset + i * relocationheader->esize))
            return 0;

        index = relocation.info >> 8;

        if (!call_read_all(descriptor, &symbol, symbolheader->esize, symbolheader->offset + index * symbolheader->esize))
            return 0;

        if (symbol.shindex)
            continue;

        address = findmodulesymbol(cstring_length(strings + symbol.name), strings + symbol.name);

        if (address)
        {

            unsigned int value;

            if (!call_read_all(descriptor, &value, 4, offset + relocation.offset))
                return 0;

            value += address;

            if (!call_write_all(descriptor, &value, 4, offset + relocation.offset))
                return 0;

        }

    }

    return 1;

}

static unsigned int resolve(unsigned int descriptor)
{

    struct elf_header header;
    unsigned int i;

    if (!readheader(descriptor, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader relocationheader;
        struct elf_sectionheader dataheader;
        struct elf_sectionheader symbolheader;
        struct elf_sectionheader stringheader;
        char strings[BUFFER_SIZE];

        if (!readsectionheader(descriptor, &header, i, &relocationheader))
            return 0;

        if (relocationheader.type != ELF_SECTION_TYPE_REL)
            continue;

        if (!readsectionheader(descriptor, &header, relocationheader.info, &dataheader))
            return 0;

        if (!readsectionheader(descriptor, &header, relocationheader.link, &symbolheader))
            return 0;

        if (!readsectionheader(descriptor, &header, symbolheader.link, &stringheader))
            return 0;

        if (stringheader.size > BUFFER_SIZE)
            return 0;

        if (!call_read_all(descriptor, strings, stringheader.size, stringheader.offset))
            return 0;

        if (!resolvesymbols(descriptor, &relocationheader, &symbolheader, strings, dataheader.offset))
            return 0;

    }

    return 1;

}

static void update(char *name, unsigned int address)
{

    char fullname[256];

    cstring_write_fmt1(fullname, 256, "%s.map\\0", 0, name);

    if (call_walk_absolute(FILE_L0, fullname))
    {

        char data[8192];
        unsigned int count = call_read(FILE_L0, data, 8192, 0);
        unsigned int offset = 0;
        unsigned int i;

        for (i = 0; (offset = buffer_eachbyte(data, count, '\n', offset)); i = offset)
        {

            if (data[i] == ' ')
                break;

            cstring_write_value(&data[i], 8, cstring_read_value(&data[i], 8, 16) + address, 16, 8, 0);

        }

        call_write_all(FILE_L0, data, count, 0);

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (!call_walk_absolute(FILE_G0, "/kernel"))
        PANIC();

    if (!call_walk_relative(FILE_G1, FILE_G0, "fudge"))
        PANIC();

    if (!call_walk_absolute(FILE_G2, mdata))
        PANIC();

    if (resolve(FILE_G2))
    {

        unsigned int address = call_load(FILE_G2);

        update(mdata, address);

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

