#include "cdfs.h"

unsigned int cdfs_read16(void *data)
{

    unsigned char *bytes = data;

    return (bytes[1] << 8) | bytes[0];

}

unsigned int cdfs_read32(void *data)
{

    unsigned char *bytes = data;

    return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

}

unsigned int cdfs_validate(struct cdfs_descriptor *descriptor)
{

    return descriptor->identifier[0] == 'C' && descriptor->identifier[1] == 'D' && descriptor->identifier[2] == '0' && descriptor->identifier[3] == '0' && descriptor->identifier[4] == '1' && descriptor->version == 1;

}

