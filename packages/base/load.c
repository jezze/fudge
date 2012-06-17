#include <fudge.h>
#include <elf.h>

#define BUFFER_SIZE 0x8000

static char buffer[BUFFER_SIZE];
static char symbolBuffer[0x1000];
static char symBuffer[0x2000];
static char strBuffer[0x2000];

unsigned int find(char *name)
{

    struct elf_header *header = (struct elf_header *)symbolBuffer;
    struct elf_section_header *sheader = (struct elf_section_header *)symbolBuffer;
    struct elf_symbol *symTable = (struct elf_symbol *)symBuffer;
    unsigned int id = call_open(FILE_NEW, "/ramdisk/boot/fudge");

    call_read(id, 0, sizeof (struct elf_header), symbolBuffer);
    call_read(id, header->shoffset, header->shsize * header->shcount, symbolBuffer);
    call_read(id, sheader[7].offset, sheader[7].size, symBuffer);
    call_read(id, sheader[sheader[7].link].offset, sheader[sheader[7].link].size, strBuffer);
    call_close(id);

    return elf_search_table(symTable, sheader[7].size / sheader[7].esize, strBuffer, name);

}

void main()
{

    struct elf_header *header = (struct elf_header *)buffer;
    unsigned int count = call_read(FILE_STDIN, 0, BUFFER_SIZE, buffer);

    elf_symbolize(header, find);
    call_write(FILE_STDIN, 0, count, buffer);
    call_load(FILE_STDIN);

}

