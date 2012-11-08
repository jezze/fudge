#define TTY_CWD_SIZE                    256

#define TTY_DRIVER_TYPE                 0x1000

struct tty_driver
{

    struct base_driver base;
    char cwdname[TTY_CWD_SIZE];
    unsigned int cwdcount;

};

struct tty_filesystem
{

    struct vfs_interface base;
    struct tty_driver *driver;

};

void tty_init_driver(struct tty_driver *driver, char *cwdname, unsigned int cwdcount);
void tty_init_filesystem(struct tty_filesystem *filesystem, struct tty_driver *driver);
