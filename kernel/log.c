#include <lib/file.h>
#include <lib/vfs.h>
#include <lib/session.h>
#include <kernel/log.h>
#include <kernel/modules.h>

void log_message(unsigned int type, char *message)
{

    file_write_string(session_get_out(), message);

}

void log_init()
{

}

