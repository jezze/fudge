#ifndef INITRD_H
#define INITRD_H

typedef struct
{

    uint32_t nfiles;

} initrd_header_t;

typedef struct
{

    uint8_t magic;
    char name[64];
    uint32_t offset;
    uint32_t length;

} initrd_file_header_t;

extern vfs_node_t *initrd_init(uint32_t location);

#endif

