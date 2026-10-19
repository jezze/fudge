#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/console.h>
#include <modules/virt/virt.h>

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

/* the tty copies what it received into memory at a physical address, which the kernel's mapping keeps equal */
static void handleirq(unsigned int irq)
{

    unsigned int count;

    while ((count = tty[REG_BYTESREADY / 4]))
    {

        unsigned int i;

        if (count > BUFFERSIZE)
            count = BUFFERSIZE;

        tty[REG_DATAPTR / 4] = (unsigned long)buffer;
        tty[REG_DATALEN / 4] = count;
        tty[REG_CMD / 4] = REG_CMD_READBUFFER;

        for (i = 0; i < count; i++)
            console_notifydata(&consoleinterface, buffer[i]);

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

    console_initinterface(&consoleinterface, id, consoleinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return id == VIRT_TTY;

}

static void driver_reset(unsigned int id)
{

    tty = (volatile unsigned int *)virt_getbase(id);

    arch_kmap(virt_getbase(id), virt_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    virt_setroutine(virt_getirq(id), handleirq);

    tty[REG_CMD / 4] = REG_CMD_INTENABLE;

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    virt_unsetroutine(virt_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "goldfish-tty", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, VIRT_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, VIRT_BUS);

}

