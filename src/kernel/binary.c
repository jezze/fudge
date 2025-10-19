#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "binary.h"

struct binary_format *binary_findformat(unsigned int base)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BINARYFORMAT)))
    {

        struct binary_format *format = current->data;

        if (format->match(base))
            return format;

    }

    return 0;

}

void binary_initformat(struct binary_format *format, unsigned int (*match)(unsigned int base), unsigned int (*findsymbol)(unsigned int base, unsigned int count, char *symbol), unsigned int (*findentry)(unsigned int base), unsigned int (*mapsection)(unsigned int base, struct mmap_entry *entry, unsigned int index))
{

    resource_init(&format->resource, RESOURCE_BINARYFORMAT, format);

    format->match = match;
    format->findsymbol = findsymbol;
    format->findentry = findentry;
    format->mapsection = mapsection;

}

