#include <fudge.h>
#include <abi.h>

static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[4096];
static unsigned int mapcount;

static unsigned int gettextsectionoffset(struct elf_header *header, struct elf_sectionheader *sectionheaders)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            return sectionheaders[i].offset;

    }

    return 0;

}

/* this function should update the mapdata by parsing the module instead of the map */
static void relocate(struct elf_header *header, struct elf_sectionheader *sectionheaders, unsigned int address)
{

    unsigned int offset = 0;
    unsigned int i;

    /* all symbols are relative to certain section. now we just assume .text but this should be fixed */
    address += gettextsectionoffset(header, sectionheaders);

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

static unsigned int loadmap(char *map, char *buffer, unsigned int count)
{

    unsigned int target = fs_auth(map);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, map);

        if (id)
            return fs_read_full(1, target, id, buffer, count, 0);

    }

    return 0;

}

static unsigned int savemap(char *map, char *buffer, unsigned int count)
{

    unsigned int target = fs_auth(map);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, map);

        if (id)
            return fs_write_all(1, target, id, buffer, count, 0);

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
                char data[4096];
                unsigned int count;

                cstring_write_fmt2(module, 32, "initrd:kernel/%w.ko.map\\0", 0, symbol, &underscore);

                count = loadmap(module, data, 4096);

                if (count)
                    address = findsymbol(data, count, length, symbol);

            }

            if (address)
            {

                mapdata[i + 9] = 'A';

                cstring_write_value(&mapdata[i], 8, address, 16, 8, 0);

            }

        }

    }

}

static void resolve(unsigned int source, unsigned int target, unsigned int id, struct elf_header *header, struct elf_sectionheader *sectionheaders)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
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
                PANIC(source);

            fs_read_all(1, target, id, strings, stringheader->size, stringheader->offset);

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf_relocation relocation;
                struct elf_symbol symbol;

                fs_read_all(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_all(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);

                if (!symbol.shindex)
                {

                    unsigned int address = findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);

                    if (address)
                    {

                        unsigned int value;

                        fs_read_all(1, target, id, &value, 4, dataheader->offset + relocation.offset);

                        value += address;

                        fs_write_all(1, target, id, &value, 4, dataheader->offset + relocation.offset);

                    }

                }

            }

        }

    }

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int target = fs_auth(mdata);
    char mapname[256];

    cstring_write_fmt1(mapname, 256, "%s.map\\0", 0, mdata);
    kernelcount = loadmap("initrd:kernel/fudge.map", kerneldata, 8192);
    mapcount = loadmap(mapname, mapdata, 4096);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, mdata);

        if (id)
        {

            struct elf_header header;

            fs_read_all(1, target, id, &header, ELF_HEADER_SIZE, 0);

            if (elf_validate(&header))
            {

                if (header.shcount < 64)
                {

                    struct elf_sectionheader sectionheaders[64];

                    fs_read_all(1, target, id, sectionheaders, header.shsize * header.shcount, header.shoffset);
                    updateundefined();
                    resolve(source, target, id, &header, sectionheaders);
                    relocate(&header, sectionheaders, call_load(fs_map(1, target, id)));
                    savemap(mapname, mapdata, mapcount);

                }

            }

        }

    }

}

void init(void)
{

    channel_bind(EVENT_PATH, onpath);

}

