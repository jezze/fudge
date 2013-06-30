struct cpio_header
{

    unsigned short magic;
    unsigned short dev;
    unsigned short ino;
    unsigned short mode;
    unsigned short uid;
    unsigned short gid;
    unsigned short nlink;
    unsigned short rdev;
    unsigned short mtime[2];
    unsigned short namesize;
    unsigned short filesize[2];

};

unsigned int cpio_validate(struct cpio_header *header);
