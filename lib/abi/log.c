#include <fudge.h>
#include "file.h"

void log(unsigned int descriptor, char *key, char *value)
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = 0;

    count += memory_write(buffer, FUDGE_BSIZE, key, ascii_length(key), count);
    count += memory_write(buffer, FUDGE_BSIZE, ": ", 2, count);
    count += memory_write(buffer, FUDGE_BSIZE, value, ascii_length(value), count);
    count += memory_write(buffer, FUDGE_BSIZE, "", 1, count);

    if (!file_walk2(descriptor, "/system/log/send"))
        return;

    file_open(descriptor);
    file_writeall(descriptor, buffer, count);
    file_close(descriptor);

}

