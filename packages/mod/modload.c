#include <fudge.h>
#include <format/elf.h>

static unsigned int find(const void *in, unsigned int count, char x)
{

    const char *ip;

    for (ip = in; count--; ip++)
    {

        if (*ip == x)
            return (unsigned int)(ip - (char *)in);

    }

    return 0;

}

static unsigned int find_symbol(unsigned int id, unsigned int count, char *symbol)
{

    struct elf_header header;
    struct elf_section_header sectionheader[16];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);

    if (!elf_validate(&header))
        return 0;

    call_read(id, header.shoffset, header.shsize * header.shcount, sectionheader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_symbol symbols[512];
        char strings[4096];
        unsigned int address;

        if (sectionheader[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        call_read(id, sectionheader[i].offset, sectionheader[i].size, symbols);
        call_read(id, sectionheader[sectionheader[i].link].offset, sectionheader[sectionheader[i].link].size, strings);

        address = elf_find_symbol(&header, sectionheader, &sectionheader[i], symbols, strings, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int find_symbol_kernel(unsigned int count, char *symbol)
{

    unsigned int address = (call_open(CALL_E0, CALL_DR, 10, "boot/fudge")) ? find_symbol(CALL_E0, count, symbol) : 0;

    call_close(CALL_E0);

    return address;

}

static unsigned int find_symbol_module(unsigned int count, char *symbol)
{

    unsigned int length = find(symbol, count, '_');
    unsigned int offset = 0;
    unsigned int address;
    char module[64];

    offset += memory_write(module, 64, "boot/mod/", 9, offset);
    offset += memory_write(module, 64, symbol, length, offset);
    offset += memory_write(module, 64, ".ko", 3, offset);
    address = (call_open(CALL_E0, CALL_DR, offset, module)) ? find_symbol(CALL_E0, count, symbol) : 0;

    call_close(CALL_E0);

    return address;

}

static unsigned int resolve_symbols(struct elf_section_header *relocationheader, struct elf_relocation *relocations, struct elf_symbol *symbols, char *strings, char *buffer)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        unsigned char index = relocations[i].info >> 8;
        unsigned int *entry = (unsigned int *)(buffer + relocations[i].offset);
        char *symbol;
        unsigned int address;

        if (symbols[index].shindex)
            continue;

        symbol = strings + symbols[index].name;
        address = find_symbol_kernel(string_length(symbol), symbol);

        if (!address)
            address = find_symbol_module(string_length(symbol), symbol);

        if (!address)
            return 0;

        *entry += address;

    }

    return 1;

}

unsigned int resolve(unsigned int id)
{

    struct elf_header header;
    struct elf_section_header sectionheader[16];
    struct elf_relocation relocations[512];
    struct elf_symbol symbols[512];
    char strings[4096];
    char buffer[8192];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);
    call_read(id, header.shoffset, header.shsize * header.shcount, sectionheader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_section_header *relocationheader;
        struct elf_section_header *dataheader;
        struct elf_section_header *symbolheader;
        struct elf_section_header *stringheader;

        if (sectionheader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relocationheader = &sectionheader[i];
        dataheader = &sectionheader[relocationheader->info];
        symbolheader = &sectionheader[relocationheader->link];
        stringheader = &sectionheader[symbolheader->link];

        call_read(id, symbolheader->offset, symbolheader->size, symbols);
        call_read(id, stringheader->offset, stringheader->size, strings);
        call_read(id, relocationheader->offset, relocationheader->size, relocations);
        call_read(id, dataheader->offset, dataheader->size, buffer);

        if (!resolve_symbols(relocationheader, relocations, symbols, strings, buffer))
            return 0;

        call_write(id, dataheader->offset, dataheader->size, buffer);

    }

    return 1;

}

void main()
{

    if (resolve(CALL_I0))
        call_load(CALL_I0);

}

