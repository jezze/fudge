#include <fudge.h>
#include <kernel.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include "virtio.h"

static unsigned long pagealign(unsigned long value)
{

    return (value + 0xFFF) & ~0xFFFUL;

}

void virtio_reset(unsigned short io)
{

    io_outb(io + VIRTIO_REG_STATUS, 0);

    io_inb(io + VIRTIO_REG_STATUS);

}

void virtio_setstatus(unsigned short io, unsigned char status)
{

    io_outb(io + VIRTIO_REG_STATUS, io_inb(io + VIRTIO_REG_STATUS) | status);

}

unsigned int virtio_negotiate(unsigned short io, unsigned int features)
{

    virtio_reset(io);
    virtio_setstatus(io, VIRTIO_STATUS_ACKNOWLEDGE);
    virtio_setstatus(io, VIRTIO_STATUS_DRIVER);

    features &= io_ind(io + VIRTIO_REG_HOSTFEATURES);

    io_outd(io + VIRTIO_REG_GUESTFEATURES, features);

    return features;

}

unsigned int virtio_setqueue(struct virtio_queue *queue, unsigned short io, unsigned short index, void *memory)
{

    unsigned long base = pagealign((unsigned long)memory);
    unsigned long used;
    unsigned short size;

    io_outw(io + VIRTIO_REG_QUEUESELECT, index);

    size = io_inw(io + VIRTIO_REG_QUEUESIZE);

    if (!size || size > VIRTIO_QUEUESIZE)
        return 0;

    used = pagealign(base + 16 * size + 6 + 2 * size);

    if (used + 6 + 8 * size > (unsigned long)memory + VIRTIO_QUEUEMEMORY)
        return 0;

    buffer_clear((void *)base, used + 6 + 8 * size - base);

    queue->io = io;
    queue->index = index;
    queue->size = size;
    queue->lastused = 0;
    queue->descs = (struct virtio_desc *)base;
    queue->avail = (struct virtio_ringhead *)(base + 16 * size);
    queue->availring = (unsigned short *)(queue->avail + 1);
    queue->used = (struct virtio_ringhead *)used;
    queue->usedring = (struct virtio_usedelement *)(queue->used + 1);

    io_outd(io + VIRTIO_REG_QUEUEADDRESS, base >> 12);

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

void virtio_notify(struct virtio_queue *queue)
{

    io_outw(queue->io + VIRTIO_REG_QUEUENOTIFY, queue->index);

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

static unsigned int setvectors(unsigned short io, unsigned int queues)
{

    unsigned short i;

    io_outw(io + VIRTIO_REG_CONFIGVECTOR, VIRTIO_NOVECTOR);

    for (i = 0; i < queues; i++)
    {

        io_outw(io + VIRTIO_REG_QUEUESELECT, i);
        io_outw(io + VIRTIO_REG_QUEUEVECTOR, 0);

        if (io_inw(io + VIRTIO_REG_QUEUEVECTOR) != 0)
            return 0;

    }

    return 1;

}

unsigned int virtio_setinterrupt(unsigned int id, unsigned short io, unsigned int queues, void (*routine)(unsigned int irq))
{

    unsigned int msi = apic_setmsiroutine(routine);

    if (msi)
    {

        if (pci_setmsix(id, 0, apic_getmsiaddress(), apic_getmsidata(msi)) && setvectors(io, queues))
            return msi;

        pci_unsetmsix(id);
        apic_unsetroutine(msi, routine);

    }

    pic_setroutine(pci_getirq(id), routine);
    apic_setroutine(pci_getirq(id), routine);

    return 0;

}

void virtio_unsetinterrupt(unsigned int id, unsigned int msi, void (*routine)(unsigned int irq))
{

    if (msi)
    {

        pci_unsetmsix(id);
        apic_unsetroutine(msi, routine);

    }

    else
    {

        pic_unsetroutine(pci_getirq(id), routine);
        apic_unsetroutine(pci_getirq(id), routine);

    }

}
