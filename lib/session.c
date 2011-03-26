#include <lib/call.h>
#include <lib/file.h>

struct file_node *session_get_cwd()
{

    struct file_node *node = call_open("/dev/cwd");

    char buffer[256];
    file_read(node, 0, 256, buffer);

    return call_open(buffer);

}

struct file_node *session_get_in()
{

    return call_open("/dev/stdin");

}

struct file_node *session_get_out()
{

    return call_open("/dev/stdout");

}

