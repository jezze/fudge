#include <fudge.h>
#include "resource.h"
#include "binary.h"

struct binary_format *binary_findformat(unsigned int physical)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_BINARYFORMAT)))
    {

        struct binary_format *format = current->data;

        if (format->match(physical))
            return format;

    }

    return 0;

}

void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int physical), unsigned int (*findsymbol)(unsigned int physical, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int physical), unsigned int (*findbase)(unsigned int physical, unsigned int address), unsigned int (*copyprogram)(unsigned int physical), unsigned int (*relocate)(unsigned int physical))
{

    resource_init(&format->resource, RESOURCE_BINARYFORMAT, format);

    format->match = match;
    format->findsymbol = findsymbol;
    format->findentry = findentry;
    format->findbase = findbase;
    format->copyprogram = copyprogram;
    format->relocate = relocate;

}

