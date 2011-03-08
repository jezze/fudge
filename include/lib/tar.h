#ifndef LIB_TAR_H
#define LIB_TAR_H

#define TAR_BLOCK 512
#define TAR_MAGIC "ustar"

#define TAR_NAME_COUNT     100
#define TAR_MODE_COUNT     8
#define TAR_UID_COUNT      8
#define TAR_GID_COUNT      8
#define TAR_SIZE_COUNT     12
#define TAR_MTIME_COUNT    12
#define TAR_CHECKSUM_COUNT 8
#define TAR_TYPEFLAG_COUNT 1
#define TAR_MAGIC_COUNT    6
#define TAR_VERSION_COUNT  2
#define TAR_UNAME_COUNT    32
#define TAR_GNAME_COUNT    32
#define TAR_DEVMAJOR_COUNT 8
#define TAR_DEVMINOR_COUNT 8
#define TAR_PREFIX_COUNT   155
#define TAR_FILL_COUNT     12

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

    char name[TAR_NAME_COUNT];
    char mode[TAR_MODE_COUNT];
    char uid[TAR_UID_COUNT];
    char gid[TAR_GID_COUNT];
    char size[TAR_SIZE_COUNT];
    char mtime[TAR_MTIME_COUNT];
    char chksum[TAR_CHECKSUM_COUNT];
    char typeflag[TAR_TYPEFLAG_COUNT];
    char linkname[TAR_NAME_COUNT];
    char magic[TAR_MAGIC_COUNT];
    char version[TAR_VERSION_COUNT];
    char uname[TAR_UNAME_COUNT];
    char gname[TAR_GNAME_COUNT];
    char devmajor[TAR_DEVMAJOR_COUNT];
    char devminor[TAR_DEVMINOR_COUNT];
    char prefix[TAR_PREFIX_COUNT];
    char fill[TAR_FILL_COUNT];

};

#endif

