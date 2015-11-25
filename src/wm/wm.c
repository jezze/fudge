#include <abi.h>
#include <fudge.h>
#include <lib/file.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

#define REMOTES                         64
#define VIEWS                           8

static struct remote
{

    struct list_item item;
    struct element_window window;
    unsigned int source;

} remote[REMOTES];

static struct view
{

    struct list remotes;
    unsigned int center;
    struct element_panel panel;
    struct element_text number;
    char *numberstring;
    struct remote *remotefocus;

} view[VIEWS];

struct client
{

    struct element_fill background;
    struct element_mouse mouse;
    struct ctrl_videosettings oldsettings;
    struct ctrl_videosettings settings;
    struct view *viewfocus;
    unsigned int keymod;
    unsigned int quit;
    struct box size;
    struct box menu;
    struct box body;

};

static struct list remotes;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct client *client, struct event_header *header, void *data);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writefill(unsigned int source, unsigned int z, struct element_fill *fill)
{

    writeelement((unsigned int)fill, ELEMENT_TYPE_FILL, source, z, sizeof (struct element_fill));

    datacount += memory_write(databuffer, FUDGE_BSIZE, fill, sizeof (struct element_fill), datacount);

}

static void writemouse(unsigned int source, unsigned int z, struct element_mouse *mouse)
{

    writeelement((unsigned int)mouse, ELEMENT_TYPE_MOUSE, source, z, sizeof (struct element_mouse));

    datacount += memory_write(databuffer, FUDGE_BSIZE, mouse, sizeof (struct element_mouse), datacount);

}

static void writepanel(unsigned int source, unsigned int z, struct element_panel *panel)
{

    writeelement((unsigned int)panel, ELEMENT_TYPE_PANEL, source, z, sizeof (struct element_panel));

    datacount += memory_write(databuffer, FUDGE_BSIZE, panel, sizeof (struct element_panel), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, unsigned int count, void *buffer)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void writewindow(unsigned int source, unsigned int z, struct element_window *window)
{

    writeelement((unsigned int)window, ELEMENT_TYPE_WINDOW, source, z, sizeof (struct element_window));

    datacount += memory_write(databuffer, FUDGE_BSIZE, window, sizeof (struct element_window), datacount);

}

static void writeremote(unsigned int source, unsigned int z, struct remote *remote)
{

    writewindow(source, z, &remote->window);

}

static void writeview(unsigned int source, unsigned int z, struct view *view)
{

    writepanel(source, z, &view->panel);
    writetext(source, z, &view->number, 1, view->numberstring);

}

static void flush(void)
{

    if (datacount)
    {

        file_writeall(CALL_PO, databuffer, datacount);

        datacount = 0;

    }

}

static void activateremote(struct remote *remote)
{

    remote->window.active = 1;

}

static void deactivateremote(struct remote *remote)
{

    remote->window.active = 0;

}

static void showremotes(unsigned int source, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        send_wmshow(CALL_L2, remote->source);
        writeremote(source, 1, remote);

    }

}

static void hideremotes(unsigned int source, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        send_wmhide(CALL_L2, remote->source);
        writeremote(source, 0, remote);

    }

}

static void arrangeview(struct view *view, struct box *body)
{

    struct list_item *current = view->remotes.tail;
    struct remote *remote;
    unsigned int y;
    unsigned int h;

    switch (view->remotes.count)
    {

    case 0:
        break;

    case 1:
        remote = current->data;

        box_setsize(&remote->window.size, body->x, body->y, body->w, body->h);
        send_wmresize(CALL_L2, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

        break;

    default:
        y = body->y;
        h = body->h / (view->remotes.count - 1);
        remote = current->data;

        box_setsize(&remote->window.size, body->x, body->y, view->center, body->h);
        send_wmresize(CALL_L2, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

        for (current = view->remotes.tail->prev; current; current = current->prev)
        {

            remote = current->data;

            box_setsize(&remote->window.size, body->x + view->center, y, body->w - view->center, h);
            send_wmresize(CALL_L2, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

            y += h;

        }

        break;

    }

}

static void activateview(struct view *view)
{

    view->panel.active = 1;
    view->number.type = ELEMENT_TEXTTYPE_HIGHLIGHT;

}

static void deactivateview(struct view *view)
{

    view->panel.active = 0;
    view->number.type = ELEMENT_TEXTTYPE_NORMAL;

}

static void showview(unsigned int source, struct view *view)
{

    activateview(view);
    writeview(source, 1, view);
    showremotes(source, &view->remotes);

}

static void hideview(unsigned int source, struct view *view)
{

    deactivateview(view);
    writeview(source, 1, view);
    hideremotes(source, &view->remotes);

}

static void onkeypress(struct client *client, struct event_header *header, void *data)
{

    struct event_keypress *keypress = data;
    struct view *nextview;
    struct remote *nextremote;

    switch (keypress->scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod |= KEYMOD_SHIFT;

        break;

    case 0x38:
        client->keymod |= KEYMOD_ALT;

        break;

    }

    if (!(client->keymod & KEYMOD_ALT))
    {

        if (client->viewfocus->remotefocus)
            send_keypress(CALL_L2, client->viewfocus->remotefocus->source, keypress->scancode);

        return;

    }

    switch (keypress->scancode)
    {

    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        nextview = &view[keypress->scancode - 0x02];

        if (nextview == client->viewfocus)
            break;

        hideview(header->destination, client->viewfocus);

        if (client->viewfocus->remotefocus && client->keymod & KEYMOD_SHIFT)
        {

            list_move(&nextview->remotes, &client->viewfocus->remotes, &client->viewfocus->remotefocus->item);

            client->viewfocus->remotefocus = (client->viewfocus->remotes.tail) ? client->viewfocus->remotes.tail->data : 0;

            if (client->viewfocus->remotefocus)
                activateremote(client->viewfocus->remotefocus);

            arrangeview(client->viewfocus, &client->body);
            arrangeview(nextview, &client->body);

            if (nextview->remotefocus)
                deactivateremote(nextview->remotefocus);

            nextview->remotefocus = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        client->viewfocus = nextview;

        showview(header->destination, client->viewfocus);

        break;

    case 0x10:
        if (!(client->keymod & KEYMOD_SHIFT))
            break;

        if (!client->viewfocus->remotefocus)
            break;

        send_wmhide(CALL_L2, client->viewfocus->remotefocus->source);
        send_wmunmap(CALL_L2, client->viewfocus->remotefocus->source);
        writewindow(header->destination, 0, &client->viewfocus->remotefocus->window);
        list_move(&remotes, &client->viewfocus->remotes, &client->viewfocus->remotefocus->item);

        client->viewfocus->remotefocus = (client->viewfocus->remotes.tail) ? client->viewfocus->remotes.tail->data : 0;

        if (client->viewfocus->remotefocus)
            activateremote(client->viewfocus->remotefocus);

        arrangeview(client->viewfocus, &client->body);
        showremotes(header->destination, &client->viewfocus->remotes);

        break;

    case 0x19:
        if (!(client->keymod & KEYMOD_SHIFT))
            break;

        if (!call_walk(CALL_CP, CALL_PR, 10, "bin/wshell"))
            break;

        call_spawn();

        break;

    case 0x1C:
        if (!client->viewfocus->remotefocus)
            break;

        list_move(&client->viewfocus->remotes, &client->viewfocus->remotes, &client->viewfocus->remotefocus->item);
        arrangeview(client->viewfocus, &client->body);
        showremotes(header->destination, &client->viewfocus->remotes);

        break;

    case 0x23:
        if (client->viewfocus->center <= 1 * (client->size.w / 8))
            break;

        client->viewfocus->center -= (client->body.w / 32);

        arrangeview(client->viewfocus, &client->body);
        showremotes(header->destination, &client->viewfocus->remotes);

        break;

    case 0x24:
        if (!client->viewfocus->remotefocus)
            break;

        nextremote = client->viewfocus->remotefocus->item.next ? client->viewfocus->remotefocus->item.next->data : client->viewfocus->remotes.head;

        if (!nextremote || nextremote == client->viewfocus->remotefocus)
            break;

        deactivateremote(client->viewfocus->remotefocus);
        writeremote(header->destination, 1, client->viewfocus->remotefocus);

        client->viewfocus->remotefocus = nextremote;

        activateremote(client->viewfocus->remotefocus);
        writeremote(header->destination, 1, client->viewfocus->remotefocus);

        break;

    case 0x25:
        if (!client->viewfocus->remotefocus)
            break;

        nextremote = client->viewfocus->remotefocus->item.prev ? client->viewfocus->remotefocus->item.prev->data : client->viewfocus->remotes.tail;

        if (!nextremote || nextremote == client->viewfocus->remotefocus)
            break;

        deactivateremote(client->viewfocus->remotefocus);
        writeremote(header->destination, 1, client->viewfocus->remotefocus);

        client->viewfocus->remotefocus = nextremote;

        activateremote(client->viewfocus->remotefocus);
        writeremote(header->destination, 1, client->viewfocus->remotefocus);

        break;

    case 0x26:
        if (client->viewfocus->center >= 7 * (client->size.w / 8))
            break;

        client->viewfocus->center += (client->body.w / 32);

        arrangeview(client->viewfocus, &client->body);
        showremotes(header->destination, &client->viewfocus->remotes);

        break;

    case 0x2C:
        if (!(client->keymod & KEYMOD_SHIFT))
            break;

        send_wmhide(CALL_L2, header->destination);
        send_wmunmap(CALL_L2, header->destination);

        break;

    }

}

static void onkeyrelease(struct client *client, struct event_header *header, void *data)
{

    struct event_keyrelease *keyrelease = data;

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        client->keymod &= ~KEYMOD_SHIFT;

        break;

    case 0x38:
        client->keymod &= ~KEYMOD_ALT;

        break;

    }

    if (!(client->keymod & KEYMOD_ALT))
    {

        if (client->viewfocus->remotefocus)
            send_keyrelease(CALL_L2, client->viewfocus->remotefocus->source, keyrelease->scancode);

        return;

    }

}

static void onmousepress(struct client *client, struct event_header *header, void *data)
{

    struct event_mousepress *mousepress = data;
    struct list_item *current;
    unsigned int i;

    switch (mousepress->button)
    {

    case 0x01:
        for (i = 0; i < VIEWS; i++)
        {

            if (!box_isinside(&view[i].panel.size, client->mouse.x, client->mouse.y))
                continue;

            if (&view[i] != client->viewfocus)
            {

                hideview(header->destination, client->viewfocus);

                client->viewfocus = &view[i];

                showview(header->destination, client->viewfocus);

            }

            return;

        }

        for (current = client->viewfocus->remotes.head; current; current = current->next)
        {

            struct remote *remote = current->data;

            if (!box_isinside(&remote->window.size, client->mouse.x, client->mouse.y))
                continue;

            if (remote != client->viewfocus->remotefocus)
            {

                if (client->viewfocus->remotefocus)
                {

                    deactivateremote(client->viewfocus->remotefocus);
                    writeremote(header->destination, 1, client->viewfocus->remotefocus);

                }

                client->viewfocus->remotefocus = remote;

                activateremote(client->viewfocus->remotefocus);
                writeremote(header->destination, 1, client->viewfocus->remotefocus);

            }

            return;

        }

        break;

    }

}

static void onmousemove(struct client *client, struct event_header *header, void *data)
{

    struct event_mousemove *mousemove = data;

    client->mouse.x += mousemove->relx;
    client->mouse.y -= mousemove->rely;

    if (mousemove->relx > 0 && client->mouse.x >= client->size.w)
        client->mouse.x = client->size.w - 1;

    if (mousemove->relx < 0 && client->mouse.x >= client->size.w)
        client->mouse.x = 0;

    if (mousemove->rely < 0 && client->mouse.y >= client->size.h)
        client->mouse.y = client->size.h - 1;

    if (mousemove->rely > 0 && client->mouse.y >= client->size.h)
        client->mouse.y = 0;

    writemouse(header->destination, 3, &client->mouse);

}

static void onwmmap(struct client *client, struct event_header *header, void *data)
{

    if (header->source == header->destination)
    {

        ctrl_setvideosettings(&client->settings, 1920, 1080, 32);
        call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
        call_open(CALL_L0);
        file_seekreadall(CALL_L0, &client->oldsettings, sizeof (struct ctrl_videosettings), 0);
        file_seekwriteall(CALL_L0, &client->settings, sizeof (struct ctrl_videosettings), 0);
        file_seekreadall(CALL_L0, &client->settings, sizeof (struct ctrl_videosettings), 0);
        call_close(CALL_L0);
        send_wmresize(CALL_L2, header->destination, 0, 0, client->settings.w, client->settings.h);
        send_wmshow(CALL_L2, header->destination);

    }
            
    else
    {

        if (!remotes.count)
            return;

        if (client->viewfocus->remotefocus)
            deactivateremote(client->viewfocus->remotefocus);

        client->viewfocus->remotefocus = remotes.head->data;
        client->viewfocus->remotefocus->source = header->source;

        list_move(&client->viewfocus->remotes, &remotes, &client->viewfocus->remotefocus->item);
        activateremote(client->viewfocus->remotefocus);
        arrangeview(client->viewfocus, &client->body);
        showremotes(header->destination, &client->viewfocus->remotes);

    }

}

static void onwmunmap(struct client *client, struct event_header *header, void *data)
{

    unsigned int i;

    if (header->source == header->destination)
    {

        call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
        call_open(CALL_L0);
        file_seekwriteall(CALL_L0, &client->oldsettings, sizeof (struct ctrl_videosettings), 0);
        call_close(CALL_L0);

    }

    for (i = 0; i < VIEWS; i++)
    {

        while (view[i].remotes.count)
        {

            struct remote *remote = view[i].remotes.head->data;

            send_wmunmap(CALL_L2, remote->source);
            list_move(&remotes, &view[i].remotes, &remote->item);

        }

    }

    client->quit = 1;

}

static void onwmresize(struct client *client, struct event_header *header, void *data)
{

    struct event_wmresize *wmresize = data;
    unsigned int i;

    box_setsize(&client->size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&client->menu, client->size.x, client->size.y, client->size.w, 32);
    box_setsize(&client->body, client->size.x, client->size.y + client->menu.h, client->size.w, client->size.h - client->menu.h);
    box_setsize(&client->background.size, client->size.x, client->size.y, client->size.w, client->size.h);

    for (i = 0; i < VIEWS; i++)
    {

        view[i].center = client->body.w / 2;

        box_setsize(&view[i].panel.size, client->menu.x + i * client->menu.w / VIEWS, client->menu.y, client->menu.w / VIEWS, client->menu.h);
        box_setsize(&view[i].number.size, view[i].panel.size.x + 12, view[i].panel.size.y + 6, view[i].panel.size.w - 24, view[i].panel.size.h - 12);
        arrangeview(&view[i], &client->body);

    }

    client->mouse.x = client->size.x + client->size.w / 4;
    client->mouse.y = client->size.y + client->size.h / 4;

}

static void onwmshow(struct client *client, struct event_header *header, void *data)
{

    unsigned int i;

    writefill(header->destination, 1, &client->background);

    for (i = 0; i < VIEWS; i++)
        writeview(header->destination, 1, &view[i]);

    writemouse(header->destination, 3, &client->mouse);
    showremotes(header->destination, &client->viewfocus->remotes);

}

static void onwmhide(struct client *client, struct event_header *header, void *data)
{

    unsigned int i;

    writefill(header->destination, 0, &client->background);

    for (i = 0; i < VIEWS; i++)
        writeview(header->destination, 0, &view[i]);

    writemouse(header->destination, 0, &client->mouse);
    hideremotes(header->destination, &client->viewfocus->remotes);

}

static void setup(struct client *client)
{

    unsigned int i;

    element_initfill(&client->background, 2);
    element_initmouse(&client->mouse);

    for (i = 0; i < REMOTES; i++)
    {

        list_inititem(&remote[i].item, &remote[i]);
        element_initwindow(&remote[i].window);
        list_add(&remotes, &remote[i].item);

    }

    for (i = 0; i < VIEWS; i++)
    {

        element_initpanel(&view[i].panel);
        element_inittext(&view[i].number, ELEMENT_TEXTTYPE_NORMAL);

        view[i].numberstring = "12345678" + i;
        view[i].remotefocus = 0;

    }

    client->viewfocus = &view[0];
    client->viewfocus->panel.active = 1;
    client->viewfocus->number.type = ELEMENT_TEXTTYPE_HIGHLIGHT;
    client->quit = 0;
    client->keymod = KEYMOD_NONE;

}

void main(void)
{

    struct client client;
    struct event_header header;
    unsigned int count;

    setup(&client);

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_MOUSEPRESS] = onmousepress;
    handlers[EVENT_MOUSEMOVE] = onmousemove;
    handlers[EVENT_WMMAP] = onwmmap;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    if (!call_walk(CALL_L1, CALL_PR, 17, "system/event/poll"))
        return;

    if (!call_walk(CALL_L2, CALL_PR, 17, "system/event/send"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L1);
    call_open(CALL_L2);
    send_wmmap(CALL_L2);

    while ((count = file_readall(CALL_L1, &header, sizeof (struct event_header))))
    {

        unsigned char data[512];

        if (header.count)
            file_readall(CALL_L1, data, header.count);

        if (handlers[header.type])
        {

            handlers[header.type](&client, &header, data);
            flush();

        }

        if (client.quit)
            break;

    }

    call_close(CALL_L2);
    call_close(CALL_L1);
    call_close(CALL_PO);

}

