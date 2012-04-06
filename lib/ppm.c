#include <lib/memory.h>
#include <lib/ppm.h>
#include <lib/string.h>

unsigned int read_line(void *buffer)
{

    char *in = (char *)buffer;

    unsigned int i;

    for (i = 0; i < 512; i++)
    {

        if (in[i] == '\n')
            return i + 1;

    }

    return 0;

}

void ppm_parse(struct ppm_header *header, void *buffer)
{

    unsigned int offset = 0;

    memory_copy(header->sign, buffer, 2);

    offset = read_line(buffer + offset);

    // COMMENT

    offset = read_line(buffer + offset);

    header->width = 20;
    header->height = 20;

    offset = read_line(buffer + offset);

    header->data = buffer + 0x34;

}

