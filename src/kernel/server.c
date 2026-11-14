#include <fudge.h>
#include "resource.h"
#include "server.h"

void server_init(struct server *server, void *data, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data), unsigned long (*locate)(unsigned int id))
{

    resource_operands_init(&server->operands, place, locate);
    resource_init(&server->resource, 0, data, &server->operands);

}

