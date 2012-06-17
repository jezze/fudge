#include <fudge.h>
#include <elf.h>

#define BUFFER_SIZE 0x8000

static char buffer[BUFFER_SIZE];
static char symbolBuffer[0x10000];
static unsigned int symbolCount;

unsigned int find(char *name)
{

    struct elf_header *header = (struct elf_header *)symbolBuffer;

    return elf_get_symbol_plain(header, name);

}

void parse()
{

    unsigned int id;

    id = call_open(FILE_NEW, "/ramdisk/boot/fudge");
    symbolCount = call_read(id, 0, 0x10000, symbolBuffer);
    call_close(id);

}

void main()
{

    unsigned int count;
    struct elf_header *header = (struct elf_header *)buffer;

    parse();

    count = call_read(FILE_STDIN, 0, BUFFER_SIZE, buffer);
    elf_symbolize(header, find);
    call_write(FILE_STDIN, 0, count, buffer);
    call_load(FILE_STDIN);

}

