#include <fudge.h>
#include <elf.h>

static unsigned int get_symbol_section(struct elf_section_header *header, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (header[i].type == ELF_SECTION_TYPE_SYMTAB)
            return i;

    }

    return 0;

}

static unsigned int get_symbol(char *symbol)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];
    unsigned int id = call_open(FILE_NEW, "/ramdisk/boot/fudge");
    unsigned int index;

    call_read(id, 0, sizeof (struct elf_header), &header);
    call_read(id, header.shoffset, header.shsize * header.shcount, &sectionHeader);
    
    index = get_symbol_section(sectionHeader, header.shcount);

    call_read(id, sectionHeader[index].offset, sectionHeader[index].size, &symbolTable);
    call_read(id, sectionHeader[sectionHeader[index].link].offset, sectionHeader[sectionHeader[index].link].size, &stringTable);
    call_close(id);

    return elf_find_symbol(&sectionHeader[index], symbolTable, stringTable, symbol);

}

static void resolve_symbols(struct elf_section_header *relHeader, struct elf_relocate *relTable, struct elf_symbol *symTable, char *strTable, char *buffer)
{

   unsigned int i;

    for (i = 0; i < relHeader->size / relHeader->esize; i++)
    {

        struct elf_relocate *relEntry = &relTable[i];
        unsigned char type = relEntry->info & 0x0F;
        unsigned char index = relEntry->info >> 8;
        struct elf_symbol *symEntry = &symTable[index];
        unsigned int *entry = (unsigned int *)(buffer + relEntry->offset);
        unsigned int value = *entry;

        if (symEntry->shindex)
            continue;

        switch (type)
        {

            case 1:

                *entry = value + get_symbol(strTable + symEntry->name);

                break;

            case 2:

                *entry = value + get_symbol(strTable + symEntry->name);

                break;

        }

    }

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
    call_read(FILE_STDIN, header.shoffset, header.shsize * header.shcount, &sectionHeader);

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

        call_read(FILE_STDIN, symbolHeader->offset, symbolHeader->size, &symbolTable);
        call_read(FILE_STDIN, stringHeader->offset, stringHeader->size, &stringTable);
        call_read(FILE_STDIN, relocateHeader->offset, relocateHeader->size, &relocateTable);
        call_read(FILE_STDIN, relocateData->offset, relocateData->size, &buffer);

        resolve_symbols(relocateHeader, relocateTable, symbolTable, stringTable, buffer);
        call_write(FILE_STDIN, relocateData->offset, relocateData->size, buffer);

    }

    call_load(FILE_STDIN);

}

