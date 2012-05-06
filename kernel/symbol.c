#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/error.h>
#include <kernel/symbol.h>
#include <kernel/modules.h>

static char buffer[SYMBOL_BUFFER_SIZE];
static unsigned int bufferCount;

unsigned int symbol_find(char *name)
{

    unsigned int start = 0;
    unsigned int i;

    for (i = 0; i < bufferCount; i++)
    {

        if (buffer[i] != '\n')
            continue;

        char *address = buffer + start;
        char *description = buffer + start + 11;

        if (memory_compare(name, description, string_length(name)))
            return string_read_num(address, 16);

        start = i + 1;

    }

    return 0;

}

void symbol_init()
{

    struct modules_filesystem *filesystem = modules_get_filesystem("/ramdisk/");

    error_assert(filesystem != 0, "Symbol table not found", __FILE__, __LINE__);

    unsigned int id = filesystem->find(filesystem, "boot/fudge.map");

    error_assert(id != 0, "Symbol table not found", __FILE__, __LINE__);

    bufferCount = filesystem->read(filesystem, id, 0, SYMBOL_BUFFER_SIZE, buffer);

    error_assert(bufferCount != 0, "Symbol table not found", __FILE__, __LINE__);

}

