#include <call.h>
#include <file.h>
#include <session.h>
#include <tar.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string(session_get_out(), "You need to supply filename.\n");

        return;

    }

    struct file_node *node = file_find(session_get_cwd(), argv[1]);

    if (!node)
    {

        file_write_string(session_get_out(), "File does not exist.\n");

        return;

    }

    char buffer[512];
    struct tar_header *header = (struct tar_header *)&buffer;

    unsigned int offset = 0;

    while (1)
    {

        file_read(node, offset, 512, header);

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        file_write_string(session_get_out(), header->name);
        file_write_string(session_get_out(), "\t ");

        unsigned int j;
        unsigned int count = 1;
        unsigned int size = 0;

        for (j = 11; j > 0; j--, count *= 8)
            size += ((header->size[j - 1] - '0') * count);

        file_write_dec(session_get_out(), size);
        file_write_string(session_get_out(), "\n");

        offset += ((size / 512) + 1) * 512;

        if (size % 512)
            offset += 512;

    }

}

