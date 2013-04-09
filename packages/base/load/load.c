#include <fudge.h>
#include <format/elf.h>

unsigned int find(const void *in, unsigned int count, char x)
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
    struct elf_section_header sectionTable[16];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);

    if (!elf_validate(&header))
        return 0;

    call_read(id, header.shoffset, header.shsize * header.shcount, sectionTable);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_symbol symbolTable[512];
        char stringTable[4096];
        unsigned int address;

        if (sectionTable[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        call_read(id, sectionTable[i].offset, sectionTable[i].size, symbolTable);
        call_read(id, sectionTable[sectionTable[i].link].offset, sectionTable[sectionTable[i].link].size, stringTable);

        address = elf_find_symbol(&header, sectionTable, &sectionTable[i], symbolTable, stringTable, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int find_symbol_kernel(unsigned int count, char *symbol)
{

    unsigned int address;

    call_open(3, FUDGE_ROOT, 10, "boot/fudge");

    address = find_symbol(3, count, symbol);

    call_close(3);

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

    call_open(3, FUDGE_ROOT, offset, module);

    address = find_symbol(3, count, symbol);

    call_close(3);

    return address;

}

static unsigned int resolve_symbols(struct elf_section_header *relocationHeader, struct elf_relocation *relocationTable, struct elf_symbol *symbolTable, char *stringTable, char *buffer)
{

    unsigned int i;

    for (i = 0; i < relocationHeader->size / relocationHeader->esize; i++)
    {

        unsigned char index = relocationTable[i].info >> 8;
        unsigned int *entry = (unsigned int *)(buffer + relocationTable[i].offset);
        char *symbol;
        unsigned int address;

        if (symbolTable[index].shindex)
            continue;

        symbol = stringTable + symbolTable[index].name;
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
    struct elf_section_header sectionTable[16];
    struct elf_relocation relocationTable[512];
    struct elf_symbol symbolTable[512];
    char stringTable[4096];
    char buffer[8192];
    unsigned int i;

    call_read(id, 0, ELF_HEADER_SIZE, &header);
    call_read(id, header.shoffset, header.shsize * header.shcount, sectionTable);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_section_header *relocationHeader;
        struct elf_section_header *relocationData;
        struct elf_section_header *symbolHeader;
        struct elf_section_header *stringHeader;

        if (sectionTable[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relocationHeader = &sectionTable[i];
        relocationData = &sectionTable[relocationHeader->info];
        symbolHeader = &sectionTable[relocationHeader->link];
        stringHeader = &sectionTable[symbolHeader->link];

        call_read(id, symbolHeader->offset, symbolHeader->size, symbolTable);
        call_read(id, stringHeader->offset, stringHeader->size, stringTable);
        call_read(id, relocationHeader->offset, relocationHeader->size, relocationTable);
        call_read(id, relocationData->offset, relocationData->size, buffer);

        if (!resolve_symbols(relocationHeader, relocationTable, symbolTable, stringTable, buffer))
            return 0;

        call_write(id, relocationData->offset, relocationData->size, buffer);

    }

    return 1;

}

void main()
{

    if (resolve(FUDGE_IN))
        call_load(FUDGE_IN);

}

