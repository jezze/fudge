#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "renderable.h"
#include "draw.h"
#include "window.h"
#include "client.h"

void client_activate(struct client *client)
{

    client->window.framecolor = WM_COLOR_ACTIVEFRAME;

}

void client_deactivate(struct client *client)
{

    client->window.framecolor = WM_COLOR_PASSIVEFRAME;

}

void client_init(struct client *client)
{

    list_inititem(&client->item, client);
    window_init(&client->window, WM_COLOR_PASSIVEFRAME);

}

