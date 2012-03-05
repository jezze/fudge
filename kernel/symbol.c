#include <lib/string.h>
#include <kernel/error.h>
#include <kernel/symbol.h>
#include <kernel/vfs.h>

static struct symbol_entry entries[SYMBOL_ENTRY_SLOTS];
static char buffer[SYMBOL_BUFFER_SIZE];

void *symbol_find(char *name)
{

    unsigned int i;

    for (i = 0; i < SYMBOL_ENTRY_SLOTS; i++)
    {

        if (!string_compare(entries[i].name, name))
            return entries[i].paddress;

    }

    return 0;

}

void symbol_init()
{

    struct vfs_filesystem *filesystem = vfs_find_filesystem("/ramdisk/");

    if (!filesystem)
        error_panic("Symbol table not found", __FILE__, __LINE__);

    unsigned int id = filesystem->find(filesystem, "boot/fudge.map");

    if (!id)
        error_panic("Symbol table not found", __FILE__, __LINE__);

    unsigned int count = filesystem->read(filesystem, id, SYMBOL_BUFFER_SIZE, buffer);

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

                string_write(entries[index].name, buffer + start + 11);
                entries[index].paddress = (void *)string_read_num(buffer + start, 16);
                index++;

                start = i + 1;

                break;

        }

    }

}

