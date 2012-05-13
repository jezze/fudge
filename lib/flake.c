#include <lib/call.h>
#include <lib/flake.h>
#include <lib/memory.h>
#include <lib/string.h>

void *flake_parse(unsigned int type, unsigned int count, void *buffer)
{

    struct flake_header *header;

    if (count < sizeof (struct flake_header))
        return 0;

    header = (struct flake_header *)buffer;

    if (!(header->type & type))
        return 0;

    if (!header->tag)
        return 0;

    return header;

}

