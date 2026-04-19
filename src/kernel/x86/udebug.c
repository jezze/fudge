#include <fudge.h>
#include <kernel.h>

#define FRAMEBUFFER                     0xB8000
#define COLSIZE                         160
#define ROWSIZE                         25
#define TOTAL                           (COLSIZE * ROWSIZE)

static unsigned char *framebuffer = (unsigned char *)FRAMEBUFFER;
static struct debug_interface debug;
static unsigned int udebug_offset = 0;
static unsigned int udebug_count = 0;

static void print(unsigned int count, char *buffer, unsigned char color)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (buffer[i] == '\n')
        {

            udebug_offset = (udebug_offset + COLSIZE) - udebug_offset % COLSIZE;

        }

        else
        {

            framebuffer[udebug_offset++] = buffer[i];
            framebuffer[udebug_offset++] = color;

        }

        if (udebug_offset >= TOTAL)
        {

            buffer_copy(framebuffer, framebuffer + COLSIZE, TOTAL - COLSIZE);
            buffer_clear(framebuffer + TOTAL - COLSIZE, COLSIZE);

            udebug_offset = TOTAL - COLSIZE;

        }

    }

}

static void debug_write(unsigned int level, unsigned int count, char *buffer, char *file, unsigned int line)
{

    if (level)
    {

        char num[32];

        print(cstring_write_value(num, 32, udebug_count, 16, 8, 0), num, 0x03);
        print(1, " ", 0x08);
        print(cstring_length(file), file, 0x0D);
        print(1, ":", 0x0F);
        print(cstring_write_value(num, 32, line, 10, 0, 0), num, 0x0D);
        print(1, "\n", 0x08);

        switch (level)
        {

        case DEBUG_CRITICAL:
            print(8, "CRITICAL", 0x40);

            break;

        case DEBUG_ERROR:
            print(8, "   ERROR", 0x40);

            break;

        case DEBUG_WARNING:
            print(8, " WARNING", 0xE0);

            break;

        case DEBUG_INFO:
            print(8, "    INFO", 0x20);

            break;

        }

        print(1, " ", 0x08);
        print(count, buffer, 0x0F);
        print(1, "\n", 0x08);

        udebug_count++;

    }

    else
    {

        print(9, "     ... ", 0x08);
        print(count, buffer, 0x07);
        print(1, "\n", 0x08);

    }

}

void udebug_setup(void)
{

    debug_initinterface(&debug, debug_write);
    debug_registerinterface(&debug);

}

