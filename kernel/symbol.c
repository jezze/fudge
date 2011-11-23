#include <lib/string.h>
#include <kernel/symbol.h>
#include <kernel/vfs.h>

static struct kernel_symbol kernelSymbols[32];
static char kernelSymbolBuffer[1024];

void *symbol_find(char *name)
{

    unsigned int i;

    for (i = 0; i < 32; i++)
    {

        if (!string_compare(kernelSymbols[i].name, name))
            return kernelSymbols[i].paddress;

    }

    return 0;

}

void symbol_init()
{

    struct vfs_node *node = vfs_find("boot", "fudge.map");
    node->read(node, 1024, kernelSymbolBuffer);

    unsigned int i;
    unsigned int start = 0;
    unsigned int index = 0;

    for (i = 0; i < 1024; i++)
    {

        switch (kernelSymbolBuffer[i])
        {

            case ' ':

                kernelSymbolBuffer[i] = '\0';

                break;

            case '\n':

                kernelSymbolBuffer[i] = '\0';

                string_write(kernelSymbols[index].name, kernelSymbolBuffer + start + 11);
                kernelSymbols[index].paddress = (void *)string_read_num(kernelSymbolBuffer + start, 16);
                index++;

                start = i + 1;

                break;

        }

    }

}

