#include <fudge.h>
#include <modules/arch/x86/io/io.h>
#include "virtio.h"

void virtio_reset(unsigned short io)
{

    io_outb(io + VIRTIO_REGISTERSTATUS, 0);
    io_inb(io + VIRTIO_REGISTERSTATUS);

}

unsigned int virtio_pagecount(unsigned int value)
{

    return (value + (BUFFER_SIZE - 1)) / BUFFER_SIZE;

}

unsigned int virtio_pagealign(unsigned int value)
{

    return virtio_pagecount(value) * BUFFER_SIZE;

}

