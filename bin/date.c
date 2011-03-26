#include <call.h>
#include <file.h>
#include <session.h>

void main(int argc, char *argv[])
{

    struct file_node *node = call_open("/dev/rtc");

    unsigned char buffer;

    file_write_string(session_get_out(), "20");
    file_read(node, 0, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_write_string(session_get_out(), "-");

    file_read(node, 1, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_write_string(session_get_out(), "-");

    file_read(node, 2, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_write_string(session_get_out(), " ");

    file_read(node, 3, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_write_string(session_get_out(), ":");

    file_read(node, 4, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_write_string(session_get_out(), ":");

    file_read(node, 5, 1, &buffer);
    file_write_bcd(session_get_out(), buffer);

    file_read(node, 3, 1, &buffer);

    if (buffer >> 4)
        file_write_string(session_get_out(), "PM\n");
    else
        file_write_string(session_get_out(), "AM\n");

}

