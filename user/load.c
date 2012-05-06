#include <fudge.h>
#include <elf.h>

static char moduleBuffer[0x8000];
static unsigned int moduleCount;
static char symbolBuffer[0x1000];
static unsigned int symbolCount;

unsigned int symbol_find(char *name)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < symbolCount; i++)
    {

        if (symbolBuffer[i] != '\n')
            continue;

        char *address = symbolBuffer + start;
        char *description = symbolBuffer + start + 11;

        if (memory_compare(name, description, string_length(name)))
        {

            char num[16];

            memory_clear(num, 16);
            memory_copy(num, address, 8);

            return string_read_num(num, 16);

        }

        start = i + 1;

    }

    return 0;

}

void main(int argc, char *argv[])
{

    unsigned int id = call_open(FILE_NEW, "/ramdisk/boot/fudge.map");
    symbolCount = call_read(id, 0, 0x1000, symbolBuffer);
    call_close(id);

    moduleCount = call_read(FILE_STDIN, 0, 0x8000, moduleBuffer);

    elf_relocate(moduleBuffer, symbol_find);

//    call_write(FILE_STDIN, 0, moduleCount, moduleBuffer);

    call_load(FILE_STDIN);

}

