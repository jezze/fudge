#define TAR_BLOCK_SIZE                  512

#define TAR_TYPEFLAG_REGULAR            '0'
#define TAR_TYPEFLAG_LINK               '1'
#define TAR_TYPEFLAG_SYMLINK            '2'
#define TAR_TYPEFLAG_CHARACTER          '3'
#define TAR_TYPEFLAG_BLOCK              '4'
#define TAR_TYPEFLAG_DIRECTORY          '5'
#define TAR_TYPEFLAG_FIFO               '6'

struct tar_header
{

    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char zero[12];

};

unsigned int tar_readvalue(char *attribute);
unsigned int tar_validate(struct tar_header *header);
unsigned int tar_next(struct tar_header *header, unsigned int offset);
