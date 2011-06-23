#include <call.h>
#include <file.h>
#include <tar.h>
#include <string.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    int cwd = file_open("/dev/cwd");

    char path[256];

    unsigned int count = file_read(cwd, 256, path);
    string_concat(path, argv[1]);

    int file = file_open(path);

    if (!file)
    {

        file_close(file);
        file_close(cwd);

        return;

    }

    file_close(file);
    file_close(cwd);

/*

    char buffer[512];
    struct tar_header *header = (struct tar_header *)&buffer;

    unsigned int offset = 0;

    while (1)
    {

        file_read(node, offset, 512, header);

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        file_write_string(FILE_STDOUT, header->name);
        file_write_string(FILE_STDOUT, "\t ");

        unsigned int j;
        unsigned int count = 1;
        unsigned int size = 0;

        for (j = 11; j > 0; j--, count *= 8)
            size += ((header->size[j - 1] - '0') * count);

        file_write_dec2(FILE_STDOUT, size);
        file_write_string(FILE_STDOUT, "\n");

        offset += ((size / 512) + 1) * 512;

        if (size % 512)
            offset += 512;

    }

*/

}

