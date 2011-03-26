#include <lib/file.h>
#include <lib/session.h>
#include <kernel/log.h>
#include <kernel/modules.h>

void log_message(unsigned int type, char *message, void **args)
{

    struct file_node *node = session_get_out();

    switch (type)
    {

        case LOG_TYPE_INFO:

            file_write_string(node, "INFO: ");

            break;

        case LOG_TYPE_WARNING:

            file_write_string(node, "WARNING: ");

            break;

        case LOG_TYPE_ERROR:

            file_write_string(node, "ERROR: ");

            break;

    }

    file_write_string_format(node, message, args);
    file_write_string(node, "\n");

}

void log_init()
{

}

