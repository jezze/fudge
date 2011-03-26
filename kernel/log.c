#include <lib/file.h>
#include <lib/vfs.h>
#include <lib/session.h>
#include <kernel/log.h>
#include <kernel/modules.h>

void log_message(unsigned int type, char *message, void **args)
{

    file_write_string_format(session_get_out(), message, args);

}

void log_init()
{

}

