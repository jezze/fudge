#include <fudge.h>
#include <elf.h>

#define BUFFER_SIZE 0x8000

static char buffer[BUFFER_SIZE];
static char symbolBuffer[0x400];
static unsigned int symbolCount;

unsigned int find(char *name)
{

    unsigned int start;
    unsigned int i;
    char *address;
    char *description;

    start = 0;

    for (i = 0; i < symbolCount; i++)
    {

        if (symbolBuffer[i] != '\n')
            continue;

        address = symbolBuffer + start;
        description = symbolBuffer + start + 11;

        if (memory_compare(name, description, string_length(name)))
            return string_read_num(address, 16);

        start = i + 1;

    }

    return 0;

}

void parse()
{

    unsigned int id;

    id = call_open(FILE_NEW, "/ramdisk/boot/fudge.map");
    symbolCount = call_read(id, 0, 0x400, symbolBuffer);
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

