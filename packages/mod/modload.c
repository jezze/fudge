#include <fudge.h>
#include <elf/elf.h>

static unsigned int find_symbol(unsigned int id, unsigned int count, char *symbol)
{

    struct elf_header header;
    struct elf_section_header sectionheader[32];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);

    if (!elf_validate(&header))
        return 0;

    if (header.shcount > 32)
        return 0;

    call_read(id, header.shoffset, header.shsize * header.shcount, sectionheader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_symbol symbols[1024];
        char strings[4096];
        unsigned int address;
        struct elf_section_header *symbolheader;
        struct elf_section_header *stringheader;

        if (sectionheader[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        symbolheader = &sectionheader[i];
        stringheader = &sectionheader[symbolheader->link];

        if (symbolheader->size > sizeof (struct elf_symbol) * 1024)
            return 0;

        if (stringheader->size > 4096)
            return 0;

        call_read(id, symbolheader->offset, symbolheader->size, symbols);
        call_read(id, stringheader->offset, stringheader->size, strings);

        address = elf_find_symbol(&header, sectionheader, symbolheader, symbols, strings, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int find_symbol_module(unsigned int count, char *symbol)
{

    unsigned int length = memory_findbyte(symbol, count, '_');
    unsigned int offset = 0;
    unsigned int address;
    char module[32];

    offset += memory_write(module, 32, symbol, length, offset);
    offset += memory_write(module, 32, ".ko", 3, offset);

    if (!call_walk(CALL_L2, CALL_L1, offset, module))
        return 0;

    call_open(CALL_L2, CALL_OPEN_READ | CALL_OPEN_WRITE);

    address = find_symbol(CALL_L2, count, symbol);

    call_close(CALL_L2);

    return address;

}

static unsigned int resolve_symbols(unsigned int id, struct elf_section_header *relocationheader, struct elf_relocation *relocations, struct elf_symbol *symbols, char *strings, unsigned int offset)
{

    unsigned int i;

    for (i = 0; i < relocationheader->size / relocationheader->esize; i++)
    {

        unsigned char index = relocations[i].info >> 8;
        unsigned int address;
        unsigned int value;
        char *symbol;
        unsigned int count;

        if (symbols[index].shindex)
            continue;

        symbol = strings + symbols[index].name;
        count = ascii_length(symbol);
        address = find_symbol(CALL_L0, count, symbol);

        if (!address)
            address = find_symbol_module(count, symbol);

        if (!address)
            return 0;

        call_read(id, offset + relocations[i].offset, 4, &value);

        value += address;

        call_write(id, offset + relocations[i].offset, 4, &value);

    }

    return 1;

}

static unsigned int resolve(unsigned int id)
{

    struct elf_header header;
    struct elf_section_header sectionheader[32];
    struct elf_relocation relocations[1024];
    struct elf_symbol symbols[1024];
    char strings[4096];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);

    if (!elf_validate(&header))
        return 0;

    if (header.shcount > 32)
        return 0;

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

        if (relocationheader->size > sizeof (struct elf_relocation) * 1024)
            return 0;

        if (symbolheader->size > sizeof (struct elf_symbol) * 1024)
            return 0;

        if (stringheader->size > 4096)
            return 0;

        call_read(id, relocationheader->offset, relocationheader->size, relocations);
        call_read(id, symbolheader->offset, symbolheader->size, symbols);
        call_read(id, stringheader->offset, stringheader->size, strings);

        if (!resolve_symbols(id, relocationheader, relocations, symbols, strings, dataheader->offset))
            return 0;

    }

    return 1;

}

void main()
{

    if (!call_walk(CALL_L0, CALL_DR, 10, "boot/fudge"))
        return;

    if (!call_walk(CALL_L1, CALL_DR, 9, "boot/mod/"))
        return;

    call_open(CALL_L0, CALL_OPEN_READ | CALL_OPEN_WRITE);
    call_open(CALL_I0, CALL_OPEN_READ | CALL_OPEN_WRITE);

    if (resolve(CALL_I0))
        call_load(CALL_I0);

    call_close(CALL_I0);
    call_close(CALL_L0);

}

