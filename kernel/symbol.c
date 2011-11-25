#include <lib/string.h>
#include <kernel/error.h>
#include <kernel/symbol.h>
#include <kernel/vfs.h>

static struct symbol_entry symbolEntries[32];
static char symbolBuffer[SYMBOL_BUFFER_SIZE];

void *symbol_find(char *name)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!string_compare(symbolEntries[i].name, name))
            return symbolEntries[i].paddress;

    }

    return 0;

}

void symbol_init()
{

    struct vfs_node *node = vfs_find("boot", "fudge.map");

    if (!node)
        error_panic("Symbol table not found", __FILE__, __LINE__);

    unsigned int count = node->read(node, SYMBOL_BUFFER_SIZE, symbolBuffer);

    unsigned int i;
    unsigned int start = 0;
    unsigned int index = 0;

    for (i = 0; i < count; i++)
    {

        switch (symbolBuffer[i])
        {

            case ' ':

                symbolBuffer[i] = '\0';

                break;

            case '\n':

                symbolBuffer[i] = '\0';

                string_write(symbolEntries[index].name, symbolBuffer + start + 11);
                symbolEntries[index].paddress = (void *)string_read_num(symbolBuffer + start, 16);
                index++;

                start = i + 1;

                break;

        }

    }

}

