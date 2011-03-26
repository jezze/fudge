#include <file.h>
#include <session.h>

void main(int argc, char *argv[])
{

    unsigned int timer;

    file_write_string(session_get_out(), "Timer: ");
    file_write_string(session_get_out(), "\n");

}

