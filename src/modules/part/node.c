#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/block/block.h>
#include "part.h"

static struct part part;

static unsigned int part_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &part.settings, sizeof (struct ctrl_partsettings), offset);

}

static unsigned int part_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(&part.settings, sizeof (struct ctrl_partsettings), buffer, count, offset);

}

static struct system_node *part_opendata(struct system_node *self, struct service_state *state)
{

    struct block_interface *interface = part_findinterface(part.settings.interface);

    return interface->data.operations.open(&interface->data, state);

}

static struct system_node *part_closedata(struct system_node *self, struct service_state *state)
{

    struct block_interface *interface = part_findinterface(part.settings.interface);

    return interface->data.operations.close(&interface->data, state);

}

static unsigned int part_readdata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct block_interface *interface = part_findinterface(part.settings.interface);

    return interface->data.operations.read(&interface->data, current, state, buffer, count, offset);

}

static unsigned int part_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct block_interface *interface = part_findinterface(part.settings.interface);

    return interface->data.operations.write(&interface->data, current, state, buffer, count, offset);

}

void module_init(void)
{

    part_init(&part);

    part.ctrl.operations.read = part_readctrl;
    part.ctrl.operations.write = part_writectrl;
    part.data.operations.open = part_opendata;
    part.data.operations.close = part_closedata;
    part.data.operations.read = part_readdata;
    part.data.operations.write = part_writedata;

}

void module_register(void)
{

    part_register(&part);

}

void module_unregister(void)
{

    part_unregister(&part);

}

