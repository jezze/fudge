#ifndef MODULES_TTY_H
#define MODULES_TTY_H

#define TTY_CHARACTER_WIDTH  80
#define TTY_CHARACTER_HEIGHT 25
#define TTY_CHARACTER_SIZE   2000
#define TTY_CWD_SIZE         256

#define TTY_DRIVER_TYPE 0x1000

struct tty_driver
{

    struct modules_driver base;
    char cwdname[TTY_CWD_SIZE];
    unsigned short cursorOffset;
    void (*clear)(struct tty_driver *self);
    void (*scroll)(struct tty_driver *self);
    void (*putc)(struct tty_driver *self, char c);

};

struct tty_filesystem
{

    struct vfs_filesystem base;
    struct tty_driver *driver;

};

void tty_driver_init(struct tty_driver *driver, char *cwdname);
void tty_filesystem_init(struct tty_filesystem *filesystem, struct tty_driver *driver);

#endif
