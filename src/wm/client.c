#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "window.h"
#include "client.h"

void client_activate(struct client *client)
{

    client->window.header.active = 1;
    client->window.base.modified = 1;

}

void client_deactivate(struct client *client)
{

    client->window.header.active = 0;
    client->window.base.modified = 1;

}

void client_init(struct client *client)
{

    list_inititem(&client->item, client);
    window_init(&client->window);

}

