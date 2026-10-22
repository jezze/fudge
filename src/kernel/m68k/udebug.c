#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "udebug.h"

static struct debug_interface debug;
static unsigned int udebug_count = 0;

#ifdef ARCH_AMIGA
/* paula's serial port takes a character with its stop bit in SERDAT once SERDATR says the buffer is empty */
static void print(unsigned int count, char *buffer)
{

    volatile unsigned short *custom = (volatile unsigned short *)ARCH_TTY_BASE;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        while (!(custom[0x18 / 2] & 0x2000));

        custom[0x30 / 2] = 0x100 | (unsigned char)buffer[i];

    }

}
#else
/* the goldfish tty takes one character per write to its first register */
static void print(unsigned int count, char *buffer)
{

    volatile unsigned int *tty = (volatile unsigned int *)ARCH_TTY_BASE;
    unsigned int i;

    for (i = 0; i < count; i++)
        tty[0] = (unsigned char)buffer[i];

}
#endif

static void debug_write(unsigned int level, unsigned int count, char *buffer, char *file, unsigned int line)
{

    if (level)
    {

        char num[32];

        print(cstring_write_value(num, 32, udebug_count, 16, 8, 0), num);
        print(1, " ");
        print(cstring_length(file), file);
        print(1, ":");
        print(cstring_write_value(num, 32, line, 10, 0, 0), num);
        print(1, "\n");

        switch (level)
        {

        case DEBUG_CRITICAL:
            print(8, "CRITICAL");

            break;

        case DEBUG_ERROR:
            print(8, "   ERROR");

            break;

        case DEBUG_WARNING:
            print(8, " WARNING");

            break;

        case DEBUG_INFO:
            print(8, "    INFO");

            break;

        }

        print(1, " ");
        print(count, buffer);
        print(1, "\n");

        udebug_count++;

    }

    else
    {

        print(9, "     ... ");
        print(count, buffer);
        print(1, "\n");

    }

}

void udebug_setup(void)
{

    debug_initinterface(&debug, debug_write);
    debug_registerinterface(&debug);

}

