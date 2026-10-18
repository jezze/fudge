#include <fudge.h>
#include <kernel.h>
#include "cpu.h"
#include "arch.h"
#include "udebug.h"

#define REG_DATA                        0x00
#define REG_LSR                         0x05
#define REG_LSR_THRE                    (1 << 5)

static struct debug_interface debug;
static unsigned int udebug_count = 0;

static void print(unsigned int count, char *buffer)
{

    volatile unsigned char *uart = (volatile unsigned char *)ARCH_UART_BASE;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        while (!(uart[REG_LSR] & REG_LSR_THRE));

        uart[REG_DATA] = buffer[i];

    }

}

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
