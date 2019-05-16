#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include <modules/arch/x86/ide/ide.h>
#include <modules/arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;
static struct event_channel reply;

static void handleirq(unsigned int irq)
{

    unsigned char status = ide_getstatus(blockinterface.id);
    unsigned char data[512];

    if (status & 1)
        return;

    ide_rblock(blockinterface.id, data);
    event_create(&reply.o, EVENT_DATA);
    event_append(&reply.o, 512, data);
    kernel_place(0, reply.o.header.target, &reply.o);
    event_reset(&reply.o);

}

static unsigned int blockinterface_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct event_blockrequest *blockrequest;

    memory_copy(&reply.i.header, buffer, count);

    blockrequest = event_getdata(&reply);

    ide_rpio28(blockinterface.id, 0, blockrequest->count / 512, blockrequest->offset / 512);

    return count;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id);

    blockinterface.data.operations.write = blockinterface_writedata;

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_ATA;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    block_registerinterface(&blockinterface);
    pic_setroutine(ide_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(ide_getirq(id));

}

void module_init(void)
{

    base_initdriver(&driver, "ata", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, IDE_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, IDE_BUS);

}

