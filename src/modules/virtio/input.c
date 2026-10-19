#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include "virtio.h"
#include "input.h"

#define CONFIG_SELECT                   0x00
#define CONFIG_SUBSELECT                0x01
#define CONFIG_SIZE                     0x02
#define CONFIG_EVBITS                   0x11

static unsigned int hasevents(struct virtio_device *device, unsigned int type)
{

    virtio_setconfig(device, CONFIG_SELECT, CONFIG_EVBITS);
    virtio_setconfig(device, CONFIG_SUBSELECT, type);

    return virtio_getconfig(device, CONFIG_SIZE) != 0;

}

/* keyboards and mice are both virtio input devices: a mouse is the one with relative axes */
unsigned int input_match(unsigned int id, unsigned int type)
{

    struct virtio_device device;

    if (!virtio_match(id, VIRTIO_TYPE_INPUT) || !virtio_setup(&device, id))
        return 0;

    return hasevents(&device, INPUT_EVREL) == (type == INPUT_EVREL);

}

/* the event queue gets one device-writable buffer per event */
unsigned int input_setup(struct virtio_device *device, struct virtio_queue *queue, unsigned int id, void *memory, struct input_event *events)
{

    unsigned int i;

    if (!virtio_setup(device, id))
        return 0;

    virtio_negotiate(device, 0);

    if (!virtio_setqueue(queue, device, 0, memory))
        return 0;

    for (i = 0; i < INPUT_EVENTS && i < queue->size; i++)
    {

        virtio_setdesc(queue, i, &events[i], sizeof (struct input_event), VIRTIO_DESC_WRITE, 0);
        virtio_push(queue, i);

    }

    return 1;

}

void input_poll(struct virtio_device *device, struct virtio_queue *queue, unsigned int msi, struct input_event *events, void (*handle)(struct input_event *event))
{

    if (!msi)
        virtio_getisr(device);

    while (virtio_pending(queue))
    {

        unsigned int length;
        unsigned int id = virtio_pop(queue, &length);

        if (id >= INPUT_EVENTS)
            continue;

        events[id].type = virtio_le16(events[id].type);
        events[id].code = virtio_le16(events[id].code);
        events[id].value = virtio_le32(events[id].value);

        handle(&events[id]);
        virtio_push(queue, id);

    }

    virtio_notify(queue);

}

