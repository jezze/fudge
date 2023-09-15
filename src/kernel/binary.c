#include <fudge.h>
#include "resource.h"
#include "binary.h"

struct binary_format *binary_findformat(struct binary_node *node)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BINARYFORMAT)))
    {

        struct binary_format *format = current->data;

        if (format->match(node))
            return format;

    }

    return 0;

}

void binary_initnode(struct binary_node *node)
{

    node->address = 0;

}

void binary_initformat(struct binary_format *format, unsigned int (*match)(struct binary_node *node), unsigned int (*findsymbol)(struct binary_node *node, unsigned int count, char *symbol), unsigned int (*findentry)(struct binary_node *node), unsigned int (*findbase)(struct binary_node *node, unsigned int address), unsigned int (*copyprogram)(struct binary_node *node), void (*relocate)(struct binary_node *node))
{

    resource_init(&format->resource, RESOURCE_BINARYFORMAT, format);

    format->match = match;
    format->findsymbol = findsymbol;
    format->findentry = findentry;
    format->findbase = findbase;
    format->copyprogram = copyprogram;
    format->relocate = relocate;

}

