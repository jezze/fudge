#include <vfs.h>
#include <file.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    file_write(argv[1]);

}

