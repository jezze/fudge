#include <fudge.h>
#include <modules/arch/x86/io/io.h>
#include "virtio.h"

static unsigned int pagecount(unsigned int value)
{

    return (value + (4096 - 1)) / 4096;

}

static unsigned int pagealign(unsigned int value)
{

    return pagecount(value) * 4096;

}

void virtio_reset(unsigned short io)
{

    io_outb(io + VIRTIO_REG_STATUS, 0);
    io_inb(io + VIRTIO_REG_STATUS);

}

void virtio_setqueue(unsigned short io, unsigned short index, struct virtio_queue *vq, unsigned int address)
{

    io_outw(io + VIRTIO_REG_QSELECT, index);

    vq->size = io_inw(io + VIRTIO_REG_QSIZE);

    if (!vq->size)
        return;

    vq->address = pagealign(address);
    vq->buffers = (struct virtio_buffer *)vq->address;
    vq->bufferssize = 16 * vq->size;
    vq->availablehead = (struct virtio_availablehead *)(vq->address + vq->bufferssize);
    vq->availablering = (struct virtio_availablering *)(vq->availablehead + 1);
    vq->availablesize = 6 + 2 * vq->size;
    vq->usedhead = (struct virtio_usedhead *)(vq->address + pagealign(vq->bufferssize + vq->availablesize));
    vq->usedring = (struct virtio_usedring *)(vq->usedhead + 1);
    vq->usedsize = 6 + 8 * vq->size;

    io_outd(io + VIRTIO_REG_QADDRESS, vq->address >> 12);

}

void virtio_setrx(unsigned short io, struct virtio_queue *vq, void *buffer)
{

    unsigned int bi = vq->numbuffers;

    vq->buffers[bi].address = (unsigned int)buffer;
    vq->buffers[bi].length = 0x4000;
    vq->buffers[bi].flags = 2;
    vq->availablering[vq->availablehead->index % vq->size].index = bi;
    vq->availablehead->index++;
    vq->numbuffers++;

    io_outw(io + VIRTIO_REG_QNOTIFY, 0);

}

