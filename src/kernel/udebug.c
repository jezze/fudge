#include <fudge.h>

static unsigned int udbg_offset = 0;
static unsigned int udbg_count = 0;

static void udbg(unsigned int level, unsigned int count, char *buffer, char *file, unsigned int line)
{

    unsigned char *framebuffer = (unsigned char *)0xB8000;
    unsigned int i;
    char message[256];

    buffer_clear(message, 256);
    cstring_write_fmt2(message, 256, 0, "[%u] %s\n", &udbg_count, buffer);

    for (i = 0; i < cstring_length(message); i++)
    {

        if (message[i] == '\n')
        {

            udbg_offset = (udbg_offset + 160) - udbg_offset % 160;

        }

        else
        {

            framebuffer[udbg_offset] = message[i];

            udbg_offset++;

            framebuffer[udbg_offset] = 0x07;

            udbg_offset++;

        }

        if (udbg_offset >= 25 * 160)
        {

            buffer_copy(framebuffer, framebuffer + 160, 24 * 160);
            buffer_clear(framebuffer + 24 * 160, 160);

            udbg_offset = 24 * 160;

        }

    }

    udbg_count++;

}

void udbg_fmt0(unsigned int level, char *fmt, char *file, unsigned int line)
{

    char buffer[256];

    buffer_clear(buffer, 256);
    udbg(level, cstring_write_fmt0(buffer, 256, 0, fmt), buffer, file, line); 

}

void udbg_fmt1(unsigned int level, char *fmt, void *arg1, char *file, unsigned int line)
{

    char buffer[256];

    buffer_clear(buffer, 256);
    udbg(level, cstring_write_fmt1(buffer, 256, 0, fmt, arg1), buffer, file, line); 

}

void udbg_fmt2(unsigned int level, char *fmt, void *arg1, void *arg2, char *file, unsigned int line)
{

    char buffer[256];

    buffer_clear(buffer, 256);
    udbg(level, cstring_write_fmt2(buffer, 256, 0, fmt, arg1, arg2), buffer, file, line); 

}

