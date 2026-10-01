#include "fat.h"

unsigned int fat_validate(struct fat *fat)
{

    struct fat32 *fat32 = (struct fat32 *)fat->extended_section;
    unsigned int size = fat->bytes_per_sector;
    unsigned int cluster = fat->sectors_per_cluster;

    if (size != 512 && size != 1024 && size != 2048 && size != 4096)
        return 0;

    if (!cluster || (cluster & (cluster - 1)))
        return 0;

    if (!fat->reserved_sector_count || !fat->table_count)
        return 0;

    return !fat->root_entry_count && !fat->table_size_16 && fat32->table_size_32 && fat32->root_cluster >= 2;

}

