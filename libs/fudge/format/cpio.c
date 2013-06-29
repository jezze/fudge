#include "../memory.h"
#include "cpio.h"

unsigned int cpio_validate(struct cpio_header *header)
{

    return memory_match(header->magic, "070701", 6);

}

