#include <lib/file.h>
#include <kernel/log.h>
#include <kernel/modules.h>

void log_message(unsigned int type, char *message, void **args)
{

    switch (type)
    {

        case LOG_TYPE_INFO:

            file_write_string2(FILE_STDOUT, "INFO: ");

            break;

        case LOG_TYPE_WARNING:

            file_write_string2(FILE_STDOUT, "WARNING: ");

            break;

        case LOG_TYPE_ERROR:

            file_write_string2(FILE_STDOUT, "ERROR: ");

            break;

    }

    file_write_string_format2(FILE_STDOUT, message, args);
    file_write_string2(FILE_STDOUT, "\n");

}

void log_init()
{

}

