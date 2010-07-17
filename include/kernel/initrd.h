#ifndef INITRD_H
#define INITRD_H

struct vfs_node;

typedef struct initrd_header
{

    unsigned int nfiles;

} initrd_header_t;

typedef struct initrd_file_header
{

    unsigned char magic;
    char name[64];
    unsigned int offset;
    unsigned int length;

} initrd_file_header_t;

extern struct vfs_node *initrd_init(unsigned int location);

#endif

