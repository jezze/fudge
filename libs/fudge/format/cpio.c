#include "../memory.h"
#include "cpio.h"

unsigned int cpio_validate(struct cpio_header *header)
{

    return header->magic == 0x71C7;

}

