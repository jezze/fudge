#include "fat.h"

unsigned int fat_read16(void *data)
{

    unsigned char *bytes = data;

    return (bytes[1] << 8) | bytes[0];

}

unsigned int fat_read32(void *data)
{

    unsigned char *bytes = data;

    return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

}

void fat_write16(void *data, unsigned int value)
{

    unsigned char *bytes = data;

    bytes[0] = value;
    bytes[1] = value >> 8;

}

void fat_write32(void *data, unsigned int value)
{

    unsigned char *bytes = data;

    bytes[0] = value;
    bytes[1] = value >> 8;
    bytes[2] = value >> 16;
    bytes[3] = value >> 24;

}

unsigned int fat_validate(struct fat *fat)
{

    struct fat32 *fat32 = (struct fat32 *)fat->extended_section;
    unsigned int size = fat_read16(&fat->bytes_per_sector);
    unsigned int cluster = fat->sectors_per_cluster;

    if (size != 512 && size != 1024 && size != 2048 && size != 4096)
        return 0;

    if (!cluster || (cluster & (cluster - 1)))
        return 0;

    if (!fat_read16(&fat->reserved_sector_count) || !fat->table_count)
        return 0;

    if (fat_read16(&fat->root_entry_count))
        return fat_read16(&fat->table_size_16) != 0;

    return !fat_read16(&fat->table_size_16) && fat_read32(&fat32->table_size_32) && fat_read32(&fat32->root_cluster) >= 2;

}

