#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/icp/icp.h>
#include "pl050.h"

#define POLLS                           0x100000
#define REG_CR                          0x00
#define REG_CR_ENABLE                   (1 << 2)
#define REG_CR_RXINTERRUPT              (1 << 4)
#define REG_STAT                        0x04
#define REG_STAT_RXFULL                 (1 << 4)
#define REG_STAT_TXEMPTY                (1 << 6)
#define REG_DATA                        0x08

static volatile unsigned int *getregs(unsigned int id)
{

    return (volatile unsigned int *)icp_getbase(id);

}

unsigned int pl050_hasdata(unsigned int id)
{

    return getregs(id)[REG_STAT / 4] & REG_STAT_RXFULL;

}

unsigned char pl050_getdata(unsigned int id)
{

    return getregs(id)[REG_DATA / 4];

}

/* waits a bounded time, so a missing device does not hang module loading */
unsigned char pl050_poll(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < POLLS; i++)
    {

        if (pl050_hasdata(id))
            return pl050_getdata(id);

    }

    return 0;

}

/* sends a byte to the device and returns its answer (0xFA when acknowledged) */
unsigned char pl050_command(unsigned int id, unsigned char value)
{

    volatile unsigned int *regs = getregs(id);
    unsigned int i;

    for (i = 0; i < POLLS && !(regs[REG_STAT / 4] & REG_STAT_TXEMPTY); i++);

    regs[REG_DATA / 4] = value;

    return pl050_poll(id);

}

void pl050_flush(unsigned int id)
{

    while (pl050_hasdata(id))
        pl050_getdata(id);

}

void pl050_enable(unsigned int id)
{

    arch_kmap(icp_getbase(id), icp_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    getregs(id)[REG_CR / 4] = REG_CR_ENABLE | REG_CR_RXINTERRUPT;

}

void pl050_disable(unsigned int id)
{

    getregs(id)[REG_CR / 4] = 0;

}
