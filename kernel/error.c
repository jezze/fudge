#include "error.h"

static struct error_status status;

void error_assert(unsigned int condition, char *message, char *file, unsigned int line)
{

    if (condition)
        return;

    error_panic(message, file, line);

}

void error_panic(char *message, char *file, unsigned int line)
{

    status.message = message;
    status.file = file;
    status.line = line;

    for (;;);

}

void error_register(unsigned int index, unsigned int value)
{

    status.registers[index] = value;

}

