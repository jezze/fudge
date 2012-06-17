#include <fudge.h>
#include <elf.h>

unsigned int find(char *name)
{

    struct elf_header header;
    struct elf_section_header sectionHeader[20];
    struct elf_symbol symbolTable[400];
    char stringTable[0x1000];
    unsigned int id = call_open(FILE_NEW, "/ramdisk/boot/fudge");

    call_read(id, 0, sizeof (struct elf_header), &header);
    call_read(id, header.shoffset, header.shsize * header.shcount, &sectionHeader);
    call_read(id, sectionHeader[7].offset, sectionHeader[7].size, &symbolTable);
    call_read(id, sectionHeader[sectionHeader[7].link].offset, sectionHeader[sectionHeader[7].link].size, &stringTable);
    call_close(id);

    return elf_search_table(symbolTable, sectionHeader[7].size / sectionHeader[7].esize, stringTable, name);

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

