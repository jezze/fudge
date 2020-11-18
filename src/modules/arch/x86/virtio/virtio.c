#include <fudge.h>
#include "virtio.h"

unsigned int virtio_pagecount(unsigned int value)
{

    return (value + (BUFFER_SIZE - 1)) / BUFFER_SIZE;

}

unsigned int virtio_pagealign(unsigned int value)
{

    return virtio_pagecount(value) * BUFFER_SIZE;

}

