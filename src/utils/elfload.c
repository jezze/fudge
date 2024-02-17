#include <fudge.h>
#include <abi.h>

static struct elf_header header;
static struct elf_sectionheader sectionheaders[64];
static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[8192];
static unsigned int mapcount;

static unsigned int gettextsectionoffset(void)
{

    unsigned int i;

    for (i = 0; i < header.shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            return sectionheaders[i].offset;

    }

    return 0;

}

/* this function should update the mapdata by parsing the module instead of the map */
static void relocate(unsigned int address)
{

    unsigned int offset = 0;
    unsigned int i;

    /* all symbols are relative to certain section. now we just assume .text but this should be fixed */
    address += gettextsectionoffset();

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

static void resolve(unsigned int descriptor)
{

    unsigned int i;

    for (i = 0; i < header.shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_REL)
        {

            struct elf_sectionheader *relocationheader = &sectionheaders[i];
            struct elf_sectionheader *dataheader = &sectionheaders[relocationheader->info];
            struct elf_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC();

            call_read_all(descriptor, strings, stringheader->size, stringheader->offset);

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf_relocation relocation;
                struct elf_symbol symbol;

                call_read_all(descriptor, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                call_read_all(descriptor, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);

                if (!symbol.shindex)
                {

                    unsigned int address = findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);

                    if (address)
                    {

                        unsigned int value;

                        call_read_all(descriptor, &value, 4, dataheader->offset + relocation.offset);

                        value += address;

                        call_write_all(descriptor, &value, 4, dataheader->offset + relocation.offset);

                    }

                }

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    char mapname[256];
    unsigned int address;

    cstring_write_fmt1(mapname, 256, "%s.map\\0", 0, mdata);

    if (!call_walk_absolute(FILE_G1, "/kernel/fudge.map"))
        PANIC();

    kernelcount = call_read(FILE_G1, kerneldata, 8192, 0);

    if (!call_walk_absolute(FILE_G2, mdata))
        PANIC();

    call_read_all(FILE_G2, &header, ELF_HEADER_SIZE, 0);

    if (!elf_validate(&header))
        PANIC();

    if (header.shcount > 64)
        PANIC();

    call_read_all(FILE_G2, sectionheaders, header.shsize * header.shcount, header.shoffset);

    if (!call_walk_absolute(FILE_G3, mapname))
        PANIC();

    mapcount = call_read(FILE_G3, mapdata, 8192, 0);

    updateundefined();
    resolve(FILE_G2);

    address = call_load(FILE_G2);

    relocate(address);
    call_write_all(FILE_G3, mapdata, mapcount, 0);

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

