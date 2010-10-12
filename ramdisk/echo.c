#include <vfs.h>
#include <write.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
        return;

    write_string(argv[1]);

}

