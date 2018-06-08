#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include <modules/arch/x86/ide/ide.h>
#include <modules/arch/x86/pic/pic.h>

static struct base_driver driver;
static struct block_interface blockinterface;

static void handleirq(unsigned int irq)
{

    unsigned char status = ide_getstatus(blockinterface.id);
    unsigned char data[512];

    if (status & 1)
        return;

    ide_rblock(blockinterface.id, data, 1);
    block_notify(&blockinterface, data, 512);

}

static unsigned int blockinterface_readdata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    if (offset > 0)
        return 0;

    count = task_read(state->task, buffer, count);

    if (!count)
    {

        ide_rpio28(blockinterface.id, 0, 512, 0);
        kernel_blocktask(state->task);

    }

    return 0;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id);

    blockinterface.data.operations.read = blockinterface_readdata;

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

