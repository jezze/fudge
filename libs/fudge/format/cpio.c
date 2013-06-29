#include "../memory.h"
#include "cpio.h"

unsigned int cpio_validate(struct cpio_header *header)
{

    unsigned char id[] = {0xC7, 0x71};

    return memory_match(header->magic, id, 2);

}

