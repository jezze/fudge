#include <fudge.h>
#include <abi.h>
#include <binary.h>

static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[4096];
static unsigned int mapcount;
static char lines[4096];
static unsigned int linescount;

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

                cstring_write_fmt(module, 32, 0, "initrd:kernel/%w.ko.map\\0", symbol, &underscore);

                count = system_feed(0, 0, data, 4096, "echo %s", module);

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

static unsigned int resolve(unsigned int source, unsigned int target, unsigned int id, struct elf_header *header, struct elf_sectionheader *sectionheaders, unsigned int base)
{

    unsigned int unresolved = 0;
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
                unsigned int addend = 0;
                unsigned int value;

                fs_read_all(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_all(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + (relocation.info >> 8) * symbolheader->esize);
                fs_read_all(1, target, id, &value, 4, dataheader->offset + relocation.offset);

                if (symbol.shindex)
                {

                    addend = base + sectionheaders[symbol.shindex].offset + symbol.value;

                }

                else
                {

                    unsigned int address = findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);

                    if (address)
                    {

                        value += address;

                    }

                    else
                    {

                        channel_send_fmt(0, source, EVENT_ERROR, "Unresolved symbol: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                }

                switch (relocation.info & 0x0F)
                {

                case ELF_RELOC_TYPE_32:
                    value += addend;

                    break;

                case ELF_RELOC_TYPE_PC32:
                    value += addend - base - dataheader->offset - relocation.offset;

                    break;

                }

                fs_write_all(1, target, id, &value, 4, dataheader->offset + relocation.offset);

            }

        }

    }

    return unresolved;

}

static void onmain(struct message *message)
{

    kernelcount = system_feed(0, 0, kerneldata, 8192, "echo initrd:kernel/fudge.map");

}

static void load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            struct elf_header header;

            fs_read_all(1, target, id, &header, ELF_HEADER_SIZE, 0);

            if (elf_validate(&header))
            {

                if (header.shcount < 64)
                {

                    struct elf_sectionheader sectionheaders[64];
                    unsigned int address = fs_map(1, target, id);

                    if (address)
                    {

                        char mapname[256];

                        cstring_write_fmt(mapname, 256, 0, "%s.map\\0", path);

                        mapcount = system_feed(0, 0, mapdata, 4096, "echo %s", mapname);

                        if (mapcount)
                        {

                            fs_read_all(1, target, id, sectionheaders, header.shsize * header.shcount, header.shoffset);
                            updateundefined();

                            if (!resolve(source, target, id, &header, sectionheaders, address))
                            {

                                relocate(&header, sectionheaders, address);
                                system_feed(mapdata, mapcount, 0, 0, "write %s", mapname);
                                call_load(address);

                            }

                        }

                    }

                }

            }

        }

    }

}

static void ondata(struct message *message)
{

    linescount += buffer_write(lines, 4096, message->data, message->length, linescount);

}

static void onterm(struct message *message)
{

    unsigned int start = 0;
    unsigned int end;

    for (end = 0; end < linescount; end++)
    {

        if (lines[end] == '\n')
        {

            lines[end] = '\0';

            if (end > start)
                load(message->source, lines + start);

            start = end + 1;

        }

    }

}

static void onpath(struct message *message)
{

    load(message->source, message->data);

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

