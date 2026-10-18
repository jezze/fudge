#define CPIO_FIELDSIZE                  8

/* newc: the fields are hexadecimal text, and names and file data start on 4-byte boundaries */
struct cpio_header
{

    char magic[6];
    char ino[CPIO_FIELDSIZE];
    char mode[CPIO_FIELDSIZE];
    char uid[CPIO_FIELDSIZE];
    char gid[CPIO_FIELDSIZE];
    char nlink[CPIO_FIELDSIZE];
    char mtime[CPIO_FIELDSIZE];
    char filesize[CPIO_FIELDSIZE];
    char devmajor[CPIO_FIELDSIZE];
    char devminor[CPIO_FIELDSIZE];
    char rdevmajor[CPIO_FIELDSIZE];
    char rdevminor[CPIO_FIELDSIZE];
    char namesize[CPIO_FIELDSIZE];
    char check[CPIO_FIELDSIZE];

};

unsigned int cpio_validate(struct cpio_header *header);
unsigned int cpio_mode(struct cpio_header *header);
unsigned int cpio_namesize(struct cpio_header *header);
unsigned int cpio_filedata(struct cpio_header *header);
unsigned int cpio_filesize(struct cpio_header *header);
unsigned int cpio_next(struct cpio_header *header);
