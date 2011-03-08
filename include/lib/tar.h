#ifndef LIB_TAR_H
#define LIB_TAR_H

#define TAR_FILETYPE_NORMAL  0
#define TAR_FILETYPE_LINK    1
#define TAR_FILETYPE_SYMLINK 2
#define TAR_FILETYPE_CHAR    3
#define TAR_FILETYPE_BLOCK   4
#define TAR_FILETYPE_DIR     5
#define TAR_FILETYPE_FIFO    6
#define TAR_FILETYPE_CONT    7

struct tar_header
{

    unsigned char name[100];
    unsigned char mode[8];
    unsigned char uid[8];
    unsigned char gid[8];
    unsigned char size[12];
    unsigned char checksum[8];
    unsigned char type;
    unsigned char linkname[100];

};

#endif

