#include <fudge.h>
#include <kernel.h>
#include "internals.h"

static struct service_backend backend;

static unsigned int backend_read(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int backend_write(struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int backend_map(struct service_state *state, unsigned int offset, unsigned int count)
{

    return 0;

}

void internals_initbackend(void)
{

    service_initbackend(&backend, 3001, backend_read, backend_write, backend_map);
    resource_register(&backend.resource);

}

