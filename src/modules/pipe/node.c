#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "pipe.h"

static struct pipe pipe;
static char data0[FUDGE_BSIZE];
static char data1[FUDGE_BSIZE];

void module_init(void)
{

    pipe_init(&pipe, FUDGE_BSIZE, data0, FUDGE_BSIZE, data1);

}

void module_register(void)
{

    pipe_register(&pipe);

}

void module_unregister(void)
{

    pipe_unregister(&pipe);

}

