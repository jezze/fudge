#ifndef KERNEL_INITRD_H
#define KERNEL_INITRD_H

struct initrd_header
{

    unsigned int nfiles;

};

struct initrd_file_header
{

    unsigned char magic;
    char name[64];
    unsigned int offset;
    unsigned int length;

};

extern void initrd_init(struct initrd_header *header);

#endif

