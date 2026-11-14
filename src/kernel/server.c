#include <fudge.h>
#include "resource.h"
#include "server.h"

void server_init(struct server *server, void *data, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data))
{

    resource_init(&server->resource, 0, data, place);

}

