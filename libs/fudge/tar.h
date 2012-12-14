#define TAR_BLOCK_SIZE                  512

#define TAR_NAME_SIZE                   100
#define TAR_MODE_SIZE                   8
#define TAR_UID_SIZE                    8
#define TAR_GID_SIZE                    8
#define TAR_SIZE_SIZE                   12
#define TAR_MTIME_SIZE                  12
#define TAR_CHECKSUM_SIZE               8
#define TAR_TYPEFLAG_SIZE               1
#define TAR_LINKNAME_SIZE               100

#define TAR_FILETYPE_NORMAL             '0'
#define TAR_FILETYPE_LINK               '1'
#define TAR_FILETYPE_SYMLINK            '2'

struct tar_header
{

    char name[TAR_NAME_SIZE];
    char mode[TAR_MODE_SIZE];
    char uid[TAR_UID_SIZE];
    char gid[TAR_GID_SIZE];
    char size[TAR_SIZE_SIZE];
    char mtime[TAR_MTIME_SIZE];
    char checksum[TAR_CHECKSUM_SIZE];
    char typeflag[TAR_TYPEFLAG_SIZE];
    char linkname[TAR_LINKNAME_SIZE];

};
