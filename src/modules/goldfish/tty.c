#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/console.h>
#include <modules/platform/platform.h>
#include <modules/platform/virt/virt.h>

#define REG_PUTCHAR                     0x00
#define REG_BYTESREADY                  0x04
#define REG_CMD                         0x08
#define REG_CMD_INTENABLE               1
#define REG_CMD_READBUFFER              3
#define REG_DATAPTR                     0x10
#define REG_DATALEN                     0x14
#define BUFFERSIZE                      64

static struct base_driver driver;
static struct console_interface consoleinterface;
static volatile unsigned int *tty;
static unsigned char buffer[BUFFERSIZE];

static void handleirq(unsigned int irq)
{

    unsigned int count;

    while ((count = tty[REG_BYTESREADY / 4]))
    {

        if (count > BUFFERSIZE)
            count = BUFFERSIZE;

        tty[REG_DATAPTR / 4] = (unsigned long)buffer;
        tty[REG_DATALEN / 4] = count;
        tty[REG_CMD / 4] = REG_CMD_READBUFFER;

        console_notifydata(&consoleinterface, buffer, count);

    }

}

static unsigned int consoleinterface_send(void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        tty[REG_PUTCHAR / 4] = b[i];

    return count;

}

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, consoleinterface_send);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == PLATFORM_BUS && id == VIRT_TTY;

}

static void driver_reset(unsigned int id)
{

    tty = (volatile unsigned int *)platform_getbase(id);

    arch_kmap(platform_getbase(id), platform_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    platform_setroutine(platform_getirq(id), handleirq);

    tty[REG_CMD / 4] = REG_CMD_INTENABLE;

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    platform_unsetroutine(platform_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "goldfish-tty", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

