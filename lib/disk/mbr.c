#include "mbr.h"

unsigned int mbr_validate(struct mbr *mbr)
{

    return (mbr->signature[0] == 0x55 && mbr->signature[1] == 0xAA);

}

