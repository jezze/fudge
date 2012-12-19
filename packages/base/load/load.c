#include <fudge.h>
#include <format/elf.h>

static unsigned int get_symbol_module(char *symbol, unsigned int count, char *module)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];
    unsigned int symbolHeaderIndex;

    if (!call_open(3, FUDGE_ROOT, count, module))
        return 0;

    call_read(3, 0, ELF_HEADER_SIZE, &header);

    if (!elf_validate(&header))
        return 0;

    call_read(3, header.shoffset, header.shsize * header.shcount, sectionHeader);

    symbolHeaderIndex = elf_find_section(&header, sectionHeader, ELF_SECTION_TYPE_SYMTAB);

    call_read(3, sectionHeader[symbolHeaderIndex].offset, sectionHeader[symbolHeaderIndex].size, symbolTable);
    call_read(3, sectionHeader[sectionHeader[symbolHeaderIndex].link].offset, sectionHeader[sectionHeader[symbolHeaderIndex].link].size, stringTable);
    call_close(3);

    return elf_find_symbol(&header, sectionHeader, symbolHeaderIndex, symbolTable, stringTable, symbol);

}

static unsigned int get_symbol(char *symbol)
{

    char buffer[64];
    unsigned int address;
    unsigned int offset = 0;
    unsigned int length = (unsigned int)((char *)memory_find(symbol, "_", string_length(symbol), 1) - symbol);

    offset += memory_write(buffer, 64, "ramdisk/mod/", 12, offset);
    offset += memory_write(buffer, 64, symbol, length, offset);
    offset += memory_write(buffer, 64, ".ko", 3, offset);

    address = get_symbol_module(symbol, offset, buffer);

    if (address)
        return address;

    return get_symbol_module(symbol, 18, "ramdisk/boot/fudge");

}

static unsigned int resolve_symbols(struct elf_section_header *relocateHeader, struct elf_relocate *relocateTable, struct elf_symbol *symbolTable, char *stringTable, char *buffer)
{

   unsigned int i;

    for (i = 0; i < relocateHeader->size / relocateHeader->esize; i++)
    {

        unsigned char index = relocateTable[i].info >> 8;
        unsigned int *entry = (unsigned int *)(buffer + relocateTable[i].offset);
        unsigned int symbol;

        if (symbolTable[index].shindex)
            continue;

        symbol = get_symbol(stringTable + symbolTable[index].name);

        if (!symbol)
            return 0;

        *entry += symbol;

    }

    return 1;

}

void main()
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_relocate relocateTable[400];
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];
    char buffer[0x4000];
    unsigned int i;

    call_read(FUDGE_IN, 0, ELF_HEADER_SIZE, &header);
    call_read(FUDGE_IN, header.shoffset, header.shsize * header.shcount, sectionHeader);

    for (i = 0; i < header.shcount; i++)
    {

        struct elf_section_header *relocateHeader;
        struct elf_section_header *relocateData;
        struct elf_section_header *symbolHeader;
        struct elf_section_header *stringHeader;

        if (sectionHeader[i].type != ELF_SECTION_TYPE_REL)
            continue;

        relocateHeader = &sectionHeader[i];
        relocateData = &sectionHeader[relocateHeader->info];
        symbolHeader = &sectionHeader[relocateHeader->link];
        stringHeader = &sectionHeader[symbolHeader->link];

        call_read(FUDGE_IN, symbolHeader->offset, symbolHeader->size, symbolTable);
        call_read(FUDGE_IN, stringHeader->offset, stringHeader->size, stringTable);
        call_read(FUDGE_IN, relocateHeader->offset, relocateHeader->size, relocateTable);
        call_read(FUDGE_IN, relocateData->offset, relocateData->size, buffer);

        if (!resolve_symbols(relocateHeader, relocateTable, symbolTable, stringTable, buffer))
            return;

        call_write(FUDGE_IN, relocateData->offset, relocateData->size, buffer);

    }

    call_load(FUDGE_IN);

}

