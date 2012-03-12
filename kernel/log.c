#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/vfs.h>

void log_write(const char *buffer, ...)
{

    struct vfs_mount *mount = vfs_find_mount("/module/");

    if (!mount)
        return;

    unsigned int id = mount->filesystem->find(mount->filesystem, "tty/stdout");

    if (!id)
        return;

    char temp[0x100];
    void **arg = (void **)&buffer + 1;

    string_write(temp, buffer, *arg);

    unsigned int count = string_length(temp) + 1;

    mount->filesystem->write(mount->filesystem, id, 0, count, temp);

}

