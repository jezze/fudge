#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include "virtio.h"

static unsigned long pagealign(unsigned long value)
{

    return (value + 0xFFF) & ~0xFFFUL;

}

/* lays the descriptors, the available ring and the page aligned used ring out in memory */
unsigned int virtio_initqueue(struct virtio_queue *queue, struct virtio_device *device, unsigned short index, unsigned short size, void *memory)
{

    unsigned long base = pagealign((unsigned long)memory);
    unsigned long used;

    if (!size || size > VIRTIO_QUEUESIZE)
        return 0;

    used = pagealign(base + 16 * size + 6 + 2 * size);

    if (used + 6 + 8 * size > (unsigned long)memory + VIRTIO_QUEUEMEMORY)
        return 0;

    buffer_clear((void *)base, used + 6 + 8 * size - base);

    queue->device = device;
    queue->index = index;
    queue->size = size;
    queue->lastused = 0;
    queue->descs = (struct virtio_desc *)base;
    queue->avail = (struct virtio_ringhead *)(base + 16 * size);
    queue->availring = (unsigned short *)(queue->avail + 1);
    queue->used = (struct virtio_ringhead *)used;
    queue->usedring = (struct virtio_usedelement *)(queue->used + 1);

    return 1;

}

void virtio_setdesc(struct virtio_queue *queue, unsigned int index, void *buffer, unsigned int length, unsigned short flags, unsigned short next)
{

    volatile struct virtio_desc *desc = &queue->descs[index];
    unsigned long address = (unsigned long)buffer;

    desc->address = address;
    desc->addresshigh = (address >> 16) >> 16;
    desc->length = length;
    desc->flags = flags;
    desc->next = next;

}

void virtio_push(struct virtio_queue *queue, unsigned short head)
{

    queue->availring[queue->avail->index % queue->size] = head;
    queue->avail->index++;

}

unsigned int virtio_pending(struct virtio_queue *queue)
{

    return queue->lastused != queue->used->index;

}

unsigned int virtio_pop(struct virtio_queue *queue, unsigned int *length)
{

    volatile struct virtio_usedelement *element = &queue->usedring[queue->lastused % queue->size];

    *length = element->length;
    queue->lastused++;

    return element->id;

}
