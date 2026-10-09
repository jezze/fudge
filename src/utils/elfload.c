#include <fudge.h>
#include <abi.h>
#include <binary.h>

static char kerneldata[8192];
static unsigned int kernelcount;
static char mapdata[4096];
static unsigned int mapcount;
static char lines[4096];
static unsigned int linescount;
static unsigned int width;

static void relocate(unsigned int address)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(mapdata, mapcount, '\n', offset)); i = offset)
    {

        if (mapdata[i + width + 1] == 'T')
            cstring_write_value(&mapdata[i], width, cstring_read_value(&mapdata[i], width, 16) + address, 16, width, 0);

    }

}

static unsigned int findsymbol(char *data, unsigned int count, unsigned int length, char *symbol)
{

    unsigned int offset = 0;
    unsigned int i;

    for (i = 0; (offset = buffer_eachbyte(data, count, '\n', offset)); i = offset)
    {

        if ((data[i + width + 1] == 'T') || (data[i + width + 1] == 'A'))
        {

            if (buffer_match(&data[i + width + 3], symbol, length))
                return cstring_read_value(&data[i], width, 16);

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

        if (mapdata[i + width + 1] == 'U')
        {

            char *symbol = &mapdata[i + width + 3];
            unsigned int length = (offset - i) - (width + 3);
            unsigned int address = findsymbol(kerneldata, kernelcount, length, symbol);

            if (!address)
            {

                unsigned int underscore = buffer_findbyte(symbol, length, '_');
                char module[32];
                char data[4096];
                unsigned int count;

                cstring_write_fmt(module, 32, 0, "initrd:kernel/%w.ko.map\\0", symbol, &underscore);

                count = fs_read_path(1, module, data, 4096);

                if (count)
                    address = findsymbol(data, count, length, symbol);

            }

            if (address)
            {

                mapdata[i + width + 1] = 'A';

                cstring_write_value(&mapdata[i], width, address, 16, width, 0);

            }

        }

    }

}

static unsigned int link32(unsigned int source, unsigned int target, unsigned int id, struct elf_header *header, unsigned int base)
{

    struct elf_sectionheader sectionheaders[64];
    unsigned int textoffset = 0;
    unsigned int unresolved = 0;
    unsigned int i;

    if (header->shcount >= 64)
        return 0;

    fs_read_all(1, target, id, sectionheaders, header->shsize * header->shcount, header->shoffset);

    for (i = 0; i < header->shcount; i++)
    {

        if (!textoffset && sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            textoffset = sectionheaders[i].offset;

    }

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

    return (unresolved) ? 0 : textoffset;

}

static unsigned int link64(unsigned int source, unsigned int target, unsigned int id, struct elf64_header *header, unsigned int base)
{

    struct elf64_sectionheader sectionheaders[64];
    unsigned int textoffset = 0;
    unsigned int unresolved = 0;
    unsigned int i;

    if (header->shcount >= 64)
        return 0;

    fs_read_all(1, target, id, sectionheaders, header->shsize * header->shcount, header->shoffset);

    for (i = 0; i < header->shcount; i++)
    {

        if (!textoffset && sectionheaders[i].type == ELF_SECTION_TYPE_PROGBITS)
            textoffset = sectionheaders[i].offset;

    }

    for (i = 0; i < header->shcount; i++)
    {

        if (sectionheaders[i].type == ELF_SECTION_TYPE_RELA)
        {

            struct elf64_sectionheader *relocationheader = &sectionheaders[i];
            struct elf64_sectionheader *dataheader = &sectionheaders[relocationheader->info];
            struct elf64_sectionheader *symbolheader = &sectionheaders[relocationheader->link];
            struct elf64_sectionheader *stringheader = &sectionheaders[symbolheader->link];
            char strings[4096];
            unsigned int j;

            if (stringheader->size > 4096)
                PANIC(source);

            fs_read_all(1, target, id, strings, stringheader->size, stringheader->offset);

            for (j = 0; j < relocationheader->size / relocationheader->esize; j++)
            {

                struct elf64_relocation relocation;
                struct elf64_symbol symbol;
                unsigned long value;
                unsigned int value32;

                fs_read_all(1, target, id, &relocation, relocationheader->esize, relocationheader->offset + j * relocationheader->esize);
                fs_read_all(1, target, id, &symbol, symbolheader->esize, symbolheader->offset + relocation.symbol * symbolheader->esize);

                if (symbol.shindex)
                {

                    value = base + sectionheaders[symbol.shindex].offset + symbol.value;

                }

                else
                {

                    value = findsymbol(mapdata, mapcount, cstring_length(strings + symbol.name), strings + symbol.name);

                    if (!value)
                    {

                        channel_send_fmt(0, source, EVENT_ERROR, "Unresolved symbol: %s\n", strings + symbol.name);

                        unresolved++;

                    }

                }

                value += relocation.addend;

                switch (relocation.type)
                {

                case ELF_RELOC64_TYPE_64:
                    fs_write_all(1, target, id, &value, 8, dataheader->offset + relocation.offset);

                    break;

                case ELF_RELOC64_TYPE_32:
                case ELF_RELOC64_TYPE_32S:
                    value32 = value;

                    fs_write_all(1, target, id, &value32, 4, dataheader->offset + relocation.offset);

                    break;

                case ELF_RELOC64_TYPE_PC32:
                case ELF_RELOC64_TYPE_PLT32:
                    value32 = value - (base + dataheader->offset + relocation.offset);

                    fs_write_all(1, target, id, &value32, 4, dataheader->offset + relocation.offset);

                    break;

                default:
                    channel_send_fmt(0, source, EVENT_ERROR, "Unsupported relocation: %s\n", strings + symbol.name);

                    unresolved++;

                    break;

                }

            }

        }

    }

    return (unresolved) ? 0 : textoffset;

}

static void onmain(struct message *message)
{

    kernelcount = fs_read_path(1, "initrd:kernel/fudge.map", kerneldata, 8192);

}

static void load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);

    if (target)
    {

        unsigned int id = fs_walk(1, target, 0, path);

        if (id)
        {

            struct elf64_header header;

            if (fs_read_all(1, target, id, &header, ELF64_HEADER_SIZE, 0) >= ELF_HEADER_SIZE && elf_validate((struct elf_header *)&header))
            {

                unsigned int class = (sizeof (void *) == 8) ? ELF_IDENTITY_CLASS_64 : ELF_IDENTITY_CLASS_32;

                if (header.identify[ELF_IDENTITY_CLASS] != class)
                {

                    channel_send_fmt(0, source, EVENT_ERROR, "Unsupported module format: %s\n", path);

                }

                else
                {

                    unsigned int address = fs_map(1, target, id);

                    if (address)
                    {

                        char mapname[256];

                        cstring_write_fmt(mapname, 256, 0, "%s.map\\0", path);

                        mapcount = fs_read_path(1, mapname, mapdata, 4096);

                        if (mapcount)
                        {

                            unsigned int textoffset;

                            updateundefined();

                            textoffset = (class == ELF_IDENTITY_CLASS_64) ? link64(source, target, id, &header, address) : link32(source, target, id, (struct elf_header *)&header, address);

                            if (textoffset)
                            {

                                relocate(address + textoffset);
                                fs_write_all(1, target, fs_walk(1, target, 0, mapname), mapdata, mapcount, 0);
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

    width = sizeof (void *) * 2;

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_TERM, onterm);

}

