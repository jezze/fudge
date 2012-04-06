#include <lib/string.h>
#include <kernel/log.h>
#include <kernel/modules.h>

void log_write(const char *buffer, ...)
{

    struct modules_filesystem *filesystem = modules_get_filesystem("/module/");

    if (!filesystem)
        return;

    unsigned int id = filesystem->find(filesystem, "tty/stdout");

    if (!id)
        return;

    char temp[0x100];
    void **arg = (void **)&buffer + 1;

    string_write(temp, buffer, *arg);

    unsigned int count = string_length(temp) + 1;

    filesystem->write(filesystem, id, 0, count, temp);

}

