#include <fudge.h>
#include <elf.h>

static unsigned int get_symbol_module(char *symbol, char *module)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_section_header *symbolHeader;
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];

    if (!call_open(3, string_length(module), module))
        return 0;

    call_read(3, 0, sizeof (struct elf_header), &header);
    call_read(3, header.shoffset, header.shsize * header.shcount, sectionHeader);
    
    symbolHeader = elf_get_section(&header, sectionHeader, ELF_SECTION_TYPE_SYMTAB);

    call_read(3, symbolHeader->offset, symbolHeader->size, symbolTable);
    call_read(3, sectionHeader[symbolHeader->link].offset, sectionHeader[symbolHeader->link].size, stringTable);
    call_close(3);

    return elf_find_symbol(&header, sectionHeader, symbolHeader, symbolTable, stringTable, symbol);

}

static unsigned int get_symbol(char *symbol)
{

    char buffer[64];
    unsigned int address;
    unsigned int plength = (unsigned int)((char *)memory_find(symbol, "_", string_length(symbol), 1) - symbol);

    memory_copy(buffer, "/ramdisk/mod/", 13);
    memory_copy(buffer + 13, symbol, plength);
    memory_copy(buffer + 13 + plength, ".ko", 4);

    address = get_symbol_module(symbol, buffer);

    if (address)
        return address;

    return get_symbol_module(symbol, "/ramdisk/boot/fudge");

}

static unsigned int resolve_symbols(struct elf_section_header *relocateHeader, struct elf_relocate *relocateTable, struct elf_symbol *symbolTable, char *stringTable, char *buffer)
{

   unsigned int i;

    for (i = 0; i < relocateHeader->size / relocateHeader->esize; i++)
    {

        struct elf_relocate *relocateEntry = &relocateTable[i];
        unsigned char type = relocateEntry->info & 0x0F;
        unsigned char index = relocateEntry->info >> 8;
        struct elf_symbol *symbolEntry = &symbolTable[index];
        unsigned int *entry = (unsigned int *)(buffer + relocateEntry->offset);
        unsigned int value = *entry;
        unsigned int symbol;

        if (symbolEntry->shindex)
            continue;

        symbol = get_symbol(stringTable + symbolEntry->name);

        if (!symbol)
            return 0;

        switch (type)
        {

            case 1:

                *entry = value + symbol;

                break;

            case 2:

                *entry = value + symbol;

                break;

        }

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

    call_read(FILE_STDIN, 0, sizeof (struct elf_header), &header);
    call_read(FILE_STDIN, header.shoffset, header.shsize * header.shcount, sectionHeader);

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

        call_read(FILE_STDIN, symbolHeader->offset, symbolHeader->size, symbolTable);
        call_read(FILE_STDIN, stringHeader->offset, stringHeader->size, stringTable);
        call_read(FILE_STDIN, relocateHeader->offset, relocateHeader->size, relocateTable);
        call_read(FILE_STDIN, relocateData->offset, relocateData->size, buffer);

        if (!resolve_symbols(relocateHeader, relocateTable, symbolTable, stringTable, buffer))
            return;

        call_write(FILE_STDIN, relocateData->offset, relocateData->size, buffer);

    }

    call_load(FILE_STDIN);

}

