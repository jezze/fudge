#include <fudge.h>
#include <modules/arch/x86/io/io.h>
#include "virtio.h"

void virtio_reset(unsigned short io)
{

    io_outb(io + VIRTIO_REGISTERSTATUS, 0);
    io_inb(io + VIRTIO_REGISTERSTATUS);

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

    io_outw(io + VIRTIO_REGISTERQNOTIFY, 0);

}

unsigned int virtio_pagecount(unsigned int value)
{

    return (value + (BUFFER_SIZE - 1)) / BUFFER_SIZE;

}

unsigned int virtio_pagealign(unsigned int value)
{

    return virtio_pagecount(value) * BUFFER_SIZE;

}

