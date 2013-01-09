#include <fudge.h>
#include <format/elf.h>

static struct elf_header kHeader;
static struct elf_section_header kSectionTable[16];

static unsigned int find_symbol(struct elf_header *header, struct elf_section_header *sectionTable, unsigned int count, char *symbol)
{

    unsigned int i;

    for (i = 0; i < header->shcount; i++)
    {

        struct elf_symbol symbolTable[512];
        char stringTable[4096];
        unsigned int address;

        if (sectionTable[i].type != ELF_SECTION_TYPE_SYMTAB)
            continue;

        call_read(3, sectionTable[i].offset, sectionTable[i].size, symbolTable);
        call_read(3, sectionTable[sectionTable[i].link].offset, sectionTable[sectionTable[i].link].size, stringTable);

        address = elf_find_symbol(header, sectionTable, &sectionTable[i], symbolTable, stringTable, count, symbol);

        if (address)
            return address;

    }

    return 0;

}

static unsigned int find_symbol_kernel(unsigned int count, char *symbol)
{

    unsigned int address;

    call_open(3, FUDGE_ROOT, 18, "ramdisk/boot/fudge");

    address = find_symbol(&kHeader, kSectionTable, count, symbol);

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
            return 0;

        *entry += address;

    }

    return 1;

}

unsigned int resolve()
{

    struct elf_header header;
    struct elf_section_header sectionTable[16];
    struct elf_relocation relocationTable[512];
    struct elf_symbol symbolTable[512];
    char stringTable[4096];
    char buffer[8192];
    unsigned int i;

    call_read(FUDGE_IN, 0, ELF_HEADER_SIZE, &header);
    call_read(FUDGE_IN, header.shoffset, header.shsize * header.shcount, sectionTable);

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

        call_read(FUDGE_IN, symbolHeader->offset, symbolHeader->size, symbolTable);
        call_read(FUDGE_IN, stringHeader->offset, stringHeader->size, stringTable);
        call_read(FUDGE_IN, relocationHeader->offset, relocationHeader->size, relocationTable);
        call_read(FUDGE_IN, relocationData->offset, relocationData->size, buffer);

        if (!resolve_symbols(relocationHeader, relocationTable, symbolTable, stringTable, buffer))
            return 0;

        call_write(FUDGE_IN, relocationData->offset, relocationData->size, buffer);

    }

    return 1;

}

void main()
{

    call_open(3, FUDGE_ROOT, 18, "ramdisk/boot/fudge");
    call_read(3, 0, ELF_HEADER_SIZE, &kHeader);

    if (!elf_validate(&kHeader))
        return;

    call_read(3, kHeader.shoffset, kHeader.shsize * kHeader.shcount, kSectionTable);
    call_open(FUDGE_IN, FUDGE_ROOT, 20, "ramdisk/mod/multi.ko");

    if (resolve())
        call_load(FUDGE_IN);

    call_open(3, FUDGE_ROOT, 17, "ramdisk/bin/initm");
    call_spawn(3);

}

