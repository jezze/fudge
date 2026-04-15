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

static void debug_write(unsigned int level, unsigned int count, char *buffer, char *file, unsigned int line)
{

    char message[DEBUG_MESSAGESIZE];
    unsigned int i;

    count = cstring_write_fmt3(message, DEBUG_MESSAGESIZE, 0, "[%u] %w\n", &udebug_count, buffer, &count);

    for (i = 0; i < count; i++)
    {

        if (message[i] == '\n')
        {

            udebug_offset = (udebug_offset + COLSIZE) - udebug_offset % COLSIZE;

        }

        else
        {

            framebuffer[udebug_offset++] = message[i];
            framebuffer[udebug_offset++] = 0x07;

        }

        if (udebug_offset >= TOTAL)
        {

            buffer_copy(framebuffer, framebuffer + COLSIZE, TOTAL - COLSIZE);
            buffer_clear(framebuffer + TOTAL - COLSIZE, COLSIZE);

            udebug_offset = TOTAL - COLSIZE;

        }

    }

    udebug_count++;

}

void udebug_setup(void)
{

    debug_initinterface(&debug, debug_write);
    debug_registerinterface(&debug);

}

