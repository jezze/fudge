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

static unsigned int find(char *name)
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

    return elf_search_table(symbolTable, sectionHeader[index].size / sectionHeader[index].esize, stringTable, name);

}

void main()
{

    char moduleBuffer[0x8000];
    struct elf_header *header = (struct elf_header *)moduleBuffer;
    unsigned int count = call_read(FILE_STDIN, 0, 0x8000, moduleBuffer);

    elf_symbolize(header, find);
    call_write(FILE_STDIN, 0, count, moduleBuffer);
    call_load(FILE_STDIN);

}

