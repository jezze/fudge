#include <fudge.h>
#include "file.h"

void log(unsigned int descriptor, char *key, char *value)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += buffer_write(buffer, BUFFER_SIZE, key, ascii_length(key), count);
    count += buffer_write(buffer, BUFFER_SIZE, ": ", 2, count);
    count += buffer_write(buffer, BUFFER_SIZE, value, ascii_length(value), count);
    count += buffer_write(buffer, BUFFER_SIZE, "", 1, count);

    if (file_walk2(descriptor, "system:log/send"))
        file_writeall(descriptor, buffer, count);

}

