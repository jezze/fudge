#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include "virtio.h"

static unsigned long pagealign(unsigned long value)
{

    return (value + 0xFFF) & ~0xFFFUL;

}

/* rings, descriptors and requests are little-endian: these convert a value read or about to be written in one access, either way */
unsigned short virtio_le16(unsigned short value)
{

    unsigned char *bytes = (unsigned char *)&value;

    return (bytes[1] << 8) | bytes[0];

}

unsigned int virtio_le32(unsigned int value)
{

    unsigned char *bytes = (unsigned char *)&value;

    return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

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

    desc->address = virtio_le32(address);
    desc->addresshigh = virtio_le32((address >> 16) >> 16);
    desc->length = virtio_le32(length);
    desc->flags = virtio_le16(flags);
    desc->next = virtio_le16(next);

}

void virtio_push(struct virtio_queue *queue, unsigned short head)
{

    unsigned short index = virtio_le16(queue->avail->index);

    queue->availring[index % queue->size] = virtio_le16(head);
    queue->avail->index = virtio_le16(index + 1);

}

/* for polled queues: a device that interrupts anyway keeps a shared level-triggered line asserted, as nothing reads its ISR */
void virtio_nointerrupt(struct virtio_queue *queue)
{

    queue->avail->flags = virtio_le16(VIRTIO_AVAIL_NOINTERRUPT);

}

unsigned int virtio_pending(struct virtio_queue *queue)
{

    return queue->lastused != virtio_le16(queue->used->index);

}

unsigned int virtio_usedlength(struct virtio_queue *queue)
{

    volatile struct virtio_usedelement *element = &queue->usedring[queue->lastused % queue->size];

    return virtio_le32(element->length);

}

unsigned int virtio_pop(struct virtio_queue *queue)
{

    volatile struct virtio_usedelement *element = &queue->usedring[queue->lastused % queue->size];

    queue->lastused++;

    return virtio_le32(element->id);

}

