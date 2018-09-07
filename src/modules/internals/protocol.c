#include <fudge.h>
#include <kernel.h>
#include "internals.h"

static struct service_protocol protocol;

static unsigned int protocol_match(struct service_backend *backend, struct service_state *state)
{

    return backend->id == 3001;

}

static unsigned int protocol_root(struct service_backend *backend, struct service_state *state)
{

    return 1;

}

static unsigned int protocol_parent(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return 1;

}

static unsigned int protocol_child(struct service_backend *backend, struct service_state *state, unsigned int id, char *path, unsigned int length)
{

    return 1;

}

static unsigned int protocol_create(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_destroy(struct service_backend *backend, struct service_state *state, unsigned int id, char *name, unsigned int length)
{

    return 0;

}

static unsigned int protocol_step(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current)
{

    return 0;

}

static unsigned int protocol_open(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int protocol_close(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return id;

}

static unsigned int protocol_read(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    struct record record;

    record.id = current;
    record.size = 4096;
    record.length = memory_read(record.name, RECORD_NAMESIZE, "buffer", 6, 0);

    return memory_read(buffer, count, &record, sizeof (struct record), offset);

}

static unsigned int protocol_write(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int current, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int protocol_seek(struct service_backend *backend, struct service_state *state, unsigned int id, unsigned int offset)
{

    return offset;

}

static unsigned int protocol_map(struct service_backend *backend, struct service_state *state, unsigned int id)
{

    return 0;

}

void internals_initprotocol(void)
{

    service_initprotocol(&protocol, 3001, protocol_match, protocol_root, protocol_parent, protocol_child, protocol_create, protocol_destroy, protocol_step, protocol_open, protocol_close, protocol_read, protocol_write, protocol_seek, protocol_map);
    resource_register(&protocol.resource);

}

