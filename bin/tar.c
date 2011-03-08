#include <call.h>
#include <file.h>
#include <session.h>
#include <tar.h>
#include <vfs.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string(session_get_out(), "You need to supply filename.\n");

        return;

    }

    struct vfs_node *node = file_find(session_get_location(), argv[1]);

    if (!node)
    {

        file_write_string(session_get_out(), "File does not exist.\n");

        return;

    }

    unsigned int i = 6;
    unsigned int offset = 0;

    while (i--)
    {

        char buffer[512];

        file_read(node, offset, 512, buffer);

        struct tar_header *header = (struct tar_header *)&buffer;

        file_write_string(session_get_out(), header->name);
        file_write_string(session_get_out(), "\t ");
        file_write(session_get_out(), 0, 12, header->size);
        file_write_string(session_get_out(), "x\t ");

        unsigned int j;
        unsigned int count = 1;
        unsigned int size = 0;

        for (j = 11; j > 0; j--, count *= 10)
            size += ((header->size[j - 1] - '0') * count);

        file_write_dec(session_get_out(), size);

        file_write_string(session_get_out(), "\n");

        unsigned int blocks = size / 512;

        if (size % 512)
            blocks++;

        offset += blocks * 512 + 512;

    }

}

