#include <abi.h>
#include <fudge.h>
#include <format/elf.h>

unsigned int findsymbol(struct elf_header *header, struct elf_sectionheader *sectionheader, struct elf_sectionheader *symbolheader, struct elf_symbol *symbols, char *strings, unsigned int count, char *symbol)
{

    unsigned int i;

    for (i = 0; i < symbolheader->size / symbolheader->esize; i++)
    {

        char *s = strings + symbols[i].name;

        if (s[count] == '\0' && memory_match(symbol, s, count))
            return (header->type == ELF_TYPE_RELOCATABLE) ? sectionheader[symbols[i].shindex].address + sectionheader[symbols[i].shindex].offset + symbols[i].value : symbols[i].value;

    }

    return 0;

}

static unsigned int find_symbol(unsigned int id, unsigned int count, char *symbol)
{

    struct elf_header header;
    struct elf_sectionheader sectionheader[32];
    unsigned int i;

    if (!call_read(id, ELF_HEADER_SIZE, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    if (header.shcount > 32)
        return 0;

    call_seek(id, header.shoffset);

    if (!call_read(id, header.shsize * header.shcount, sectionheader))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_symbol symbols[1024];
        char strings[FUDGE_BSIZE];
        unsigned int address;
        struct elf_sectionheader *symbolheader;
        struct elf_sectionheader *stringheader;

        if (sectionheader[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        symbolheader = &sectionheader[i];
        stringheader = &sectionheader[symbolheader->link];

        if (symbolheader->size > sizeof (struct elf_symbol) * 1024)
            return 0;

        if (stringheader->size > FUDGE_BSIZE)
            return 0;

        call_seek(id, symbolheader->offset);

        if (!call_read(id, symbolheader->size, symbols))
            return 0;

        call_seek(id, stringheader->offset);

        if (!call_read(id, stringheader->size, strings))
            return 0;

        address = findsymbol(&header, sectionheader, symbolheader, symbols, strings, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int find_symbol_kernel(unsigned int count, char *symbol)
{

    unsigned int address;

    call_open(CALL_L0);

    address = find_symbol(CALL_L0, count, symbol);

    call_close(CALL_L0);

    return address;

}

static unsigned int find_symbol_module(unsigned int count, char *symbol)
{

    unsigned int length = memory_findbyte(symbol, count, '_') - 1;
    unsigned int offset = 0;
    unsigned int address;
    char module[32];

    offset += memory_write(module, 32, symbol, length, offset);
    offset += memory_write(module, 32, ".ko", 3, offset);

    if (!call_walk(CALL_L2, CALL_L1, offset, module))
        return 0;

    call_open(CALL_L2);

    address = find_symbol(CALL_L2, count, symbol);

    call_close(CALL_L2);

    return address;

}

static unsigned int resolve_symbols(unsigned int id, struct elf_sectionheader *relocationheader, struct elf_sectionheader *symbolheader, char *strings, unsigned int offset)
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
        address = find_symbol_module(count, symbolname);

        if (!address)
            address = find_symbol_kernel(count, symbolname);

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
    struct elf_sectionheader sectionheader[32];
    char strings[FUDGE_BSIZE];
    unsigned int i;

    if (!call_read(id, ELF_HEADER_SIZE, &header))
        return 0;

    if (!elf_validate(&header))
        return 0;

    if (header.shcount > 32)
        return 0;

    call_seek(id, header.shoffset);

    if (!call_read(id, header.shsize * header.shcount, sectionheader))
        return 0;

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_sectionheader *relocationheader;
        struct elf_sectionheader *dataheader;
        struct elf_sectionheader *symbolheader;
        struct elf_sectionheader *stringheader;

        if (sectionheader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relocationheader = &sectionheader[i];
        dataheader = &sectionheader[relocationheader->info];
        symbolheader = &sectionheader[relocationheader->link];
        stringheader = &sectionheader[symbolheader->link];

        if (stringheader->size > FUDGE_BSIZE)
            return 0;

        call_seek(id, stringheader->offset);

        if (!call_read(id, stringheader->size, strings))
            return 0;

        if (!resolve_symbols(id, relocationheader, symbolheader, strings, dataheader->offset))
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

    call_open(CALL_P0);

    if (resolve(CALL_P0))
        call_load(CALL_P0);

    call_close(CALL_P0);

}

