#include <fudge.h>
#include <elf.h>

#define BUFFER_SIZE 0x8000

static char buffer[BUFFER_SIZE];
static char symbolBuffer[0x10000];

unsigned int find(char *name)
{

    struct elf_header *header = (struct elf_header *)symbolBuffer;
    unsigned int address = (unsigned int)header;
    struct elf_section_header *sheader = (struct elf_section_header *)(address + header->shoffset);
    struct elf_section_header *symHeader = &sheader[7];
    struct elf_symbol *symTable = (struct elf_symbol *)(address + symHeader->offset);
    char *strTable = (char *)(address + sheader[symHeader->link].offset);

    return elf_search_table(symTable, symHeader->size / symHeader->esize, strTable, name);

}

void main()
{

    unsigned int id;
    unsigned int count;
    struct elf_header *header = (struct elf_header *)buffer;

    id = call_open(FILE_NEW, "/ramdisk/boot/fudge");
    call_read(id, 0, 0x10000, symbolBuffer);
    call_close(id);

    count = call_read(FILE_STDIN, 0, BUFFER_SIZE, buffer);
    elf_symbolize(header, find);
    call_write(FILE_STDIN, 0, count, buffer);
    call_load(FILE_STDIN);

}

