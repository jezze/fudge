#include <file.h>
#include <session.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    file_write_string(session_get_out(), argv[1]);

}

