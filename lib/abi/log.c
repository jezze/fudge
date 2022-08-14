#include <fudge.h>
#include "file.h"

void log(unsigned int descriptor, char *key, char *value)
{

    char buffer[BUFFER_SIZE];
    unsigned int count = 0;

    count += cstring_write(buffer, BUFFER_SIZE, key, count);
    count += cstring_write(buffer, BUFFER_SIZE, ": ", count);
    count += cstring_write(buffer, BUFFER_SIZE, value, count);
    count += cstring_writez(buffer, BUFFER_SIZE, "", count);

    if (file_walk2(descriptor, "system:log/send"))
        file_writeall(descriptor, buffer, count);

}

