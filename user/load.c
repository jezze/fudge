#include <fudge.h>
#include <elf.h>

static char moduleBuffer[0x8000];
static unsigned int moduleCount;
static char symbolBuffer[0x1000];
static unsigned int symbolCount;

unsigned int symbol_find(char *name)
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

void main()
{

    unsigned int id;

    id = call_open(FILE_NEW, "/ramdisk/boot/fudge.map");
    symbolCount = call_read(id, 0, 0x1000, symbolBuffer);
    call_close(id);

    moduleCount = call_read(FILE_STDIN, 0, 0x8000, moduleBuffer);

//    elf_relocate(moduleBuffer, symbol_find);

//    call_write(FILE_STDIN, 0, moduleCount, moduleBuffer);

    call_load(FILE_STDIN);

}

