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

static void resolve_symbols(struct elf_header *header, unsigned int (*get_symbol)(char *symbol))
{

    unsigned int address = (unsigned int)header;
    struct elf_section_header *sheader = (struct elf_section_header *)(address + header->shoffset);
    struct elf_section_header *relHeader = &sheader[2];
    struct elf_section_header *relData = &sheader[relHeader->info];
    struct elf_section_header *symHeader = &sheader[relHeader->link];
    struct elf_section_header *strHeader = &sheader[symHeader->link];
    struct elf_relocate *relTable = (struct elf_relocate *)(address + relHeader->offset);
    struct elf_symbol *symTable = (struct elf_symbol *)(address + symHeader->offset);
    unsigned int i;

    for (i = 0; i < relHeader->size / relHeader->esize; i++)
    {

        struct elf_relocate *relEntry = &relTable[i];
        unsigned char type = relEntry->info & 0x0F;
        unsigned char index = relEntry->info >> 8;
        struct elf_symbol *symEntry = &symTable[index];
        unsigned int *entry = (unsigned int *)(address + relData->offset + relEntry->offset);
        unsigned int value = *entry;

        if (symEntry->shindex)
            continue;

        switch (type)
        {

            case 1:

                *entry = value + get_symbol((char *)(address + strHeader->offset + symEntry->name));

                break;

            case 2:

                *entry = value + get_symbol((char *)(address + strHeader->offset + symEntry->name));

                break;

        }

    }

}

void main()
{

    char moduleBuffer[0x8000];
    struct elf_header *header = (struct elf_header *)moduleBuffer;
    unsigned int count = call_read(FILE_STDIN, 0, 0x8000, moduleBuffer);

    resolve_symbols(header, get_symbol);
    call_write(FILE_STDIN, 0, count, moduleBuffer);
    call_load(FILE_STDIN);

}

