#include <call.h>
#include <file.h>
#include <tar.h>

void main(int argc, char *argv[])
{

    if (argc != 2)
    {

        file_write_string2(FILE_STDOUT, "You need to supply filename.\n");

        return;

    }

    int cwd = call_open2("/dev/cwd");

    char path[256];

    unsigned int count = call_read(cwd, path, 256);
    string_concat(path, argv[1]);

    int file = call_open2(path);

    if (!file)
    {

        call_close(file);
        call_close(cwd);

        return;

    }

    call_close(file);
    call_close(cwd);

/*

    char buffer[512];
    struct tar_header *header = (struct tar_header *)&buffer;

    unsigned int offset = 0;

    while (1)
    {

        file_read(node, offset, 512, header);

        if (header->name[0] == '\0' || header->name[0] == ' ')
            break;

        file_write_string2(FILE_STDOUT, header->name);
        file_write_string2(FILE_STDOUT, "\t ");

        unsigned int j;
        unsigned int count = 1;
        unsigned int size = 0;

        for (j = 11; j > 0; j--, count *= 8)
            size += ((header->size[j - 1] - '0') * count);

        file_write_dec2(FILE_STDOUT, size);
        file_write_string2(FILE_STDOUT, "\n");

        offset += ((size / 512) + 1) * 512;

        if (size % 512)
            offset += 512;

    }
*/
}

