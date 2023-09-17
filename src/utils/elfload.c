#include <fudge.h>
#include <abi.h>

static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[8192];
static unsigned int mapcount;

static unsigned int readheader(unsigned int descriptor, struct elf_header *header)
{

    return call_read_all(descriptor, header, ELF_HEADER_SIZE, 0);

}

static unsigned int readsectionheader(unsigned int descriptor, struct elf_header *header, unsigned int index, struct elf_sectionheader *sectionheader)
{

    return call_read_all(descriptor, sectionheader, header->shsize, header->shoffset + index * header->shsize);

}

static void relocate(unsigned int address)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + 9] == 'T')
            cstring_write_value(&mapdata[i], 8, cstring_read_value(&mapdata[i], 8, 16) + address, 16, 8, 0);

    }

}

static unsigned int findsymbol(char *data, unsigned int count, unsigned int length, char *symbol)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(data, count, '\n', offset)); i = offset)
    {

        if ((data[i + 9] == 'T') || (data[i + 9] == 'A'))
        {

            if (buffer_match(&data[i + 11], symbol, length))
                return cstring_read_value(&data[i], 8, 16);

        }

    }

    return 0;

}

static void updateundefined(void)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + 9] == 'U')
        {

            char *symbol = &mapdata[i + 11];
            unsigned int length = (offset - i) - 11;
            unsigned int address = findsymbol(kerneldata, kernelcount, length, symbol);

            if (!address)
            {

                unsigned int underscore = buffer_findbyte(symbol, length, '_');
                char module[32];

                cstring_write_fmt2(module, 32, "/kernel/%w.ko.map\\0", 0, symbol, &underscore);

                if (call_walk_absolute(FILE_L0, module))
                {

                    char data[8192];
                    unsigned int count = call_read(FILE_L0, data, 8192, 0);

                    address = findsymbol(data, count, length, symbol);

                }

            }

            if (address)
            {

                mapdata[i + 9] = 'A';

                cstring_write_value(&mapdata[i], 8, address, 16, 8, 0);

            }

        }

    }

}

static void resolvesymbols(unsigned int descriptor, struct elf_sectionheader *relocationheader, struct elf_sectionheader *symbolheader, char *strings, unsigned int offset)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        struct elf_relocation relocation;
        struct elf_symbol symbol;

        call_read_all(descriptor, &relocation, relocationheader->esize, relocationheader->offset + i * relocationheader->esize);
        call_read_all(descriptor, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);

        if (!symbol.shindex)
        {

            unsigned int address = findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);

            if (address)
            {

                unsigned int value;

                call_read_all(descriptor, &value, 4, offset + relocation.offset);

                value += address;

                call_write_all(descriptor, &value, 4, offset + relocation.offset);

            }

        }

    }

}

static unsigned int gettextsectionoffset(unsigned int descriptor)
{

    struct elf_header header;
    unsigned int i;

    readheader(descriptor, &header);

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader sectionheader;

        readsectionheader(descriptor, &header, i, &sectionheader);

        if (sectionheader.type != ELF_SECTION_TYPE_PROGBITS)
            continue;

        return sectionheader.offset;

    }

    return 0;

}

static unsigned int resolve(unsigned int descriptor)
{

    struct elf_header header;
    unsigned int i;

    readheader(descriptor, &header);

    if (!elf_validate(&header))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader relocationheader;
        struct elf_sectionheader dataheader;
        struct elf_sectionheader symbolheader;
        struct elf_sectionheader stringheader;
        char strings[BUFFER_SIZE];

        readsectionheader(descriptor, &header, i, &relocationheader);

        if (relocationheader.type != ELF_SECTION_TYPE_REL)
            continue;

        readsectionheader(descriptor, &header, relocationheader.info, &dataheader);
        readsectionheader(descriptor, &header, relocationheader.link, &symbolheader);
        readsectionheader(descriptor, &header, symbolheader.link, &stringheader);

        if (stringheader.size > BUFFER_SIZE)
            return 0;

        call_read_all(descriptor, strings, stringheader.size, stringheader.offset);
        resolvesymbols(descriptor, &relocationheader, &symbolheader, strings, dataheader.offset);

    }

    return 1;

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    char mapname[256];

    cstring_write_fmt1(mapname, 256, "%s.map\\0", 0, mdata);

    if (!call_walk_absolute(FILE_G1, "/kernel/fudge.map"))
        PANIC();

    if (!call_walk_absolute(FILE_G2, mdata))
        PANIC();

    if (!call_walk_absolute(FILE_G3, mapname))
        PANIC();

    kernelcount = call_read(FILE_G1, kerneldata, 8192, 0);
    mapcount = call_read(FILE_G3, mapdata, 8192, 0);

    updateundefined();

    if (resolve(FILE_G2))
    {

        unsigned int address = call_load(FILE_G2);

        if (address)
        {

            address += gettextsectionoffset(FILE_G2);

            relocate(address);

        }

    }

    call_write_all(FILE_G3, mapdata, mapcount, 0);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

