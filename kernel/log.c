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

    unsigned int count = string_length(buffer) + 1;

    filesystem->write(filesystem, id, 0, count, (void *)buffer);

}

