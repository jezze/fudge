#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/error.h>
#include <kernel/symbol.h>
#include <kernel/modules.h>

static struct symbol_entry entries[SYMBOL_ENTRY_SLOTS];
static char buffer[SYMBOL_BUFFER_SIZE];

unsigned int symbol_find(char *name)
{

    unsigned int i;

    for (i = 0; i < SYMBOL_ENTRY_SLOTS; i++)
    {

        if (!memory_compare(entries[i].name, name, string_length(entries[i].name)))
            return entries[i].paddress;

    }

    return 0;

}

void symbol_init()
{

    struct modules_filesystem *filesystem = modules_get_filesystem("/ramdisk/");

    error_assert(filesystem != 0, "Symbol table not found", __FILE__, __LINE__);

    unsigned int id = filesystem->find(filesystem, "boot/fudge.map");

    error_assert(id != 0, "Symbol table not found", __FILE__, __LINE__);

    unsigned int count = filesystem->read(filesystem, id, 0, SYMBOL_BUFFER_SIZE, buffer);

    unsigned int i;
    unsigned int start = 0;
    unsigned int index = 0;

    for (i = 0; i < count; i++)
    {

        switch (buffer[i])
        {

            case ' ':

                buffer[i] = '\0';

                break;

            case '\n':

                buffer[i] = '\0';

                string_write(entries[index].name, "%s", buffer + start + 11);
                entries[index].paddress = string_read_num(buffer + start, 16);
                index++;

                start = i + 1;

                break;

        }

    }

}

