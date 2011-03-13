#ifndef LIB_TAR_H
#define LIB_TAR_H

#define TAR_BLOCK_SIZE 512
#define TAR_MAGIC "ustar"

#define TAR_NAME_SIZE     100
#define TAR_MODE_SIZE     8
#define TAR_UID_SIZE      8
#define TAR_GID_SIZE      8
#define TAR_SIZE_SIZE     12
#define TAR_MTIME_SIZE    12
#define TAR_CHECKSUM_SIZE 8
#define TAR_TYPEFLAG_SIZE 1
#define TAR_MAGIC_SIZE    6
#define TAR_VERSION_SIZE  2
#define TAR_UNAME_SIZE    32
#define TAR_GNAME_SIZE    32
#define TAR_DEVMAJOR_SIZE 8
#define TAR_DEVMINOR_SIZE 8
#define TAR_PREFIX_SIZE   155
#define TAR_FILL_SIZE     12

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

    char name[TAR_NAME_SIZE];
    char mode[TAR_MODE_SIZE];
    char uid[TAR_UID_SIZE];
    char gid[TAR_GID_SIZE];
    char size[TAR_SIZE_SIZE];
    char mtime[TAR_MTIME_SIZE];
    char chksum[TAR_CHECKSUM_SIZE];
    char typeflag[TAR_TYPEFLAG_SIZE];
    char linkname[TAR_NAME_SIZE];
    char magic[TAR_MAGIC_SIZE];
    char version[TAR_VERSION_SIZE];
    char uname[TAR_UNAME_SIZE];
    char gname[TAR_GNAME_SIZE];
    char devmajor[TAR_DEVMAJOR_SIZE];
    char devminor[TAR_DEVMINOR_SIZE];
    char prefix[TAR_PREFIX_SIZE];
    char fill[TAR_FILL_SIZE];

};

#endif

