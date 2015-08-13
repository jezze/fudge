#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "window.h"
#include "client.h"

void client_init(struct client *client)
{

    list_inititem(&client->item, client);
    window_init(&client->window);

}

