#include <fudge.h>
#include <format/elf.h>

static struct elf_header kHeader;
static struct elf_section_header kSectionHeader[20];
static struct elf_symbol kSymbolTable[400];
static char kStringTable[0x1000];
static unsigned int kSymbolHeaderIndex;

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

        symbol = elf_find_symbol(&kHeader, kSectionHeader, kSymbolHeaderIndex, kSymbolTable, kStringTable, stringTable + symbolTable[index].name);

        if (!symbol)
            return 0;

        *entry += symbol;

    }

    return 1;

}

unsigned int resolve()
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
            return 0;

        call_write(FUDGE_IN, relocateData->offset, relocateData->size, buffer);

    }

    return 1;

}

void main()
{

    call_open(3, FUDGE_ROOT, 18, "ramdisk/boot/fudge");
    call_read(3, 0, ELF_HEADER_SIZE, &kHeader);

    if (!elf_validate(&kHeader))
        return;

    call_read(3, kHeader.shoffset, kHeader.shsize * kHeader.shcount, kSectionHeader);

    kSymbolHeaderIndex = elf_find_section(&kHeader, kSectionHeader, ELF_SECTION_TYPE_SYMTAB);

    call_read(3, kSectionHeader[kSymbolHeaderIndex].offset, kSectionHeader[kSymbolHeaderIndex].size, kSymbolTable);
    call_read(3, kSectionHeader[kSectionHeader[kSymbolHeaderIndex].link].offset, kSectionHeader[kSectionHeader[kSymbolHeaderIndex].link].size, kStringTable);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "ramdisk/mod/base.ko");

    if (resolve())
        call_load(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 20, "ramdisk/mod/multi.ko");

    if (resolve())
        call_load(FUDGE_IN);

    call_open(3, FUDGE_ROOT, 17, "ramdisk/bin/initm");
    call_spawn(3);

}

