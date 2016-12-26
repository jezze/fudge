#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "send.h"
#include "keymap.h"
#include "ev.h"

#define REMOTES                         64
#define VIEWS                           8

static struct remote
{

    struct list_item item;
    struct element_window window;
    unsigned int source;

} remotes[REMOTES];

static struct view
{

    struct list remotes;
    unsigned int center;
    struct element_panel panel;
    struct element_text number;
    char numberstring;
    struct remote *remotefocus;

} views[VIEWS];

static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct box size;
static struct box body;
static struct list remotelist;
static struct element_fill background;
static struct element_mouse mouse;
static struct view *viewfocus = &views[0];
static struct ev_handlers handlers;

static void printinsertremote(unsigned int source, struct remote *remote)
{

    print_insertwindow(&output, source, &remote->window, 1);

}

static void printremoveremote(unsigned int source, struct remote *remote)
{

    print_removewindow(&output, source, &remote->window);

}

static void printinsertview(unsigned int source, struct view *view)
{

    print_insertpanel(&output, source, &view->panel, 1);
    print_inserttext(&output, source, &view->number, 1, 1);
    ring_write(&output, &view->numberstring, 1);

}

static void printremoveview(unsigned int source, struct view *view)
{

    print_removepanel(&output, source, &view->panel);
    print_removetext(&output, source, &view->number);

}

static void activateremote(struct remote *remote)
{

    remote->window.active = 1;

}

static void deactivateremote(struct remote *remote)
{

    remote->window.active = 0;

}

static void resizeremote(struct remote *remote, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box_setsize(&remote->window.size, x, y, w, h);
    send_wmresize(CALL_L1, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

}

static void showremotes(unsigned int source, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        send_wmshow(CALL_L1, remote->source);
        printinsertremote(source, remote);

    }

}

static void hideremotes(unsigned int source, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        send_wmhide(CALL_L1, remote->source);
        printremoveremote(source, remote);

    }

}

static void arrangeview(unsigned int source, struct view *view)
{

    struct list_item *current = view->remotes.tail;
    unsigned int y;
    unsigned int h;

    switch (view->remotes.count)
    {

    case 0:
        break;

    case 1:
        resizeremote(current->data, body.x, body.y, body.w, body.h);

        break;

    default:
        y = body.y;
        h = body.h / (view->remotes.count - 1);

        resizeremote(current->data, body.x, body.y, view->center, body.h);

        for (current = view->remotes.tail->prev; current; current = current->prev)
        {

            resizeremote(current->data, body.x + view->center, y, body.w - view->center, h);

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
    printinsertview(source, view);
    showremotes(source, &view->remotes);

}

static void hideview(unsigned int source, struct view *view)
{

    deactivateview(view);
    printinsertview(source, view);
    hideremotes(source, &view->remotes);

}

static void onkeypress(struct event_header *header, struct event_keypress *keypress)
{

    struct view *nextview;
    struct remote *nextremote;

    switch (keypress->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x38:
        keymod |= KEYMOD_ALT;

        break;

    }

    if (!(keymod & KEYMOD_ALT))
    {

        if (viewfocus->remotefocus)
            send_keypress(CALL_L2, viewfocus->remotefocus->source, keypress->scancode);

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
        nextview = &views[keypress->scancode - 0x02];

        if (nextview == viewfocus)
            break;

        hideview(header->destination, viewfocus);

        if (viewfocus->remotefocus && keymod & KEYMOD_SHIFT)
        {

            list_move(&nextview->remotes, &viewfocus->remotefocus->item);

            viewfocus->remotefocus = (viewfocus->remotes.tail) ? viewfocus->remotes.tail->data : 0;

            if (viewfocus->remotefocus)
                activateremote(viewfocus->remotefocus);

            arrangeview(header->destination, viewfocus);
            arrangeview(header->destination, nextview);

            if (nextview->remotefocus)
                deactivateremote(nextview->remotefocus);

            nextview->remotefocus = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        viewfocus = nextview;

        showview(header->destination, viewfocus);

        break;

    case 0x10:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!viewfocus->remotefocus)
            break;

        send_wmhide(CALL_L1, viewfocus->remotefocus->source);
        send_wmunmap(CALL_L1, viewfocus->remotefocus->source);
        printremoveremote(header->destination, viewfocus->remotefocus);
        list_move(&remotelist, &viewfocus->remotefocus->item);

        viewfocus->remotefocus = (viewfocus->remotes.tail) ? viewfocus->remotes.tail->data : 0;

        if (viewfocus->remotefocus)
            activateremote(viewfocus->remotefocus);

        arrangeview(header->destination, viewfocus);
        showremotes(header->destination, &viewfocus->remotes);

        break;

    case 0x19:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!file_walk(CALL_CP, "/bin/wshell"))
            break;

        call_spawn();

        break;

    case 0x1C:
        if (!viewfocus->remotefocus)
            break;

        list_move(&viewfocus->remotes, &viewfocus->remotefocus->item);
        arrangeview(header->destination, viewfocus);
        showremotes(header->destination, &viewfocus->remotes);

        break;

    case 0x23:
        if (viewfocus->center <= 1 * (body.w / 8))
            break;

        viewfocus->center -= (body.w / 32);

        arrangeview(header->destination, viewfocus);
        showremotes(header->destination, &viewfocus->remotes);

        break;

    case 0x24:
        if (!viewfocus->remotefocus)
            break;

        nextremote = viewfocus->remotefocus->item.next ? viewfocus->remotefocus->item.next->data : viewfocus->remotes.head;

        if (!nextremote || nextremote == viewfocus->remotefocus)
            break;

        deactivateremote(viewfocus->remotefocus);
        printinsertremote(header->destination, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printinsertremote(header->destination, viewfocus->remotefocus);

        break;

    case 0x25:
        if (!viewfocus->remotefocus)
            break;

        nextremote = viewfocus->remotefocus->item.prev ? viewfocus->remotefocus->item.prev->data : viewfocus->remotes.tail;

        if (!nextremote || nextremote == viewfocus->remotefocus)
            break;

        deactivateremote(viewfocus->remotefocus);
        printinsertremote(header->destination, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printinsertremote(header->destination, viewfocus->remotefocus);

        break;

    case 0x26:
        if (viewfocus->center >= 7 * (body.w / 8))
            break;

        viewfocus->center += (body.w / 32);

        arrangeview(header->destination, viewfocus);
        showremotes(header->destination, &viewfocus->remotes);

        break;

    case 0x2C:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        send_wmhide(CALL_L1, header->destination);
        send_wmunmap(CALL_L1, header->destination);

        break;

    }

}

static void onkeyrelease(struct event_header *header, struct event_keyrelease *keyrelease)
{

    switch (keyrelease->scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    case 0x38:
        keymod &= ~KEYMOD_ALT;

        break;

    }

    if (!(keymod & KEYMOD_ALT))
    {

        if (viewfocus->remotefocus)
            send_keyrelease(CALL_L2, viewfocus->remotefocus->source, keyrelease->scancode);

        return;

    }

}

static void onmousemove(struct event_header *header, struct event_mousemove *mousemove)
{

    mouse.x += mousemove->relx;
    mouse.y -= mousemove->rely;

    if (mousemove->relx > 0 && mouse.x >= size.w)
        mouse.x = size.w - 1;

    if (mousemove->relx < 0 && mouse.x >= size.w)
        mouse.x = 0;

    if (mousemove->rely < 0 && mouse.y >= size.h)
        mouse.y = size.h - 1;

    if (mousemove->rely > 0 && mouse.y >= size.h)
        mouse.y = 0;

    print_insertmouse(&output, header->destination, &mouse, 3);

}

static void onmousepress(struct event_header *header, struct event_mousepress *mousepress)
{

    struct list_item *current;
    unsigned int i;

    switch (mousepress->button)
    {

    case 0x01:
        for (i = 0; i < VIEWS; i++)
        {

            if (!box_isinside(&views[i].panel.size, mouse.x, mouse.y))
                continue;

            if (&views[i] == viewfocus)
                return;

            hideview(header->destination, viewfocus);

            viewfocus = &views[i];

            showview(header->destination, viewfocus);

        }

        for (current = viewfocus->remotes.head; current; current = current->next)
        {

            struct remote *remote = current->data;

            if (!box_isinside(&remote->window.size, mouse.x, mouse.y))
                continue;

            if (remote == viewfocus->remotefocus)
                return;

            deactivateremote(viewfocus->remotefocus);
            printinsertremote(header->destination, viewfocus->remotefocus);

            viewfocus->remotefocus = remote;

            activateremote(viewfocus->remotefocus);
            printinsertremote(header->destination, viewfocus->remotefocus);

        }

        break;

    }

}

static void onwmmap(struct event_header *header)
{

    if (header->source == header->destination)
    {

        /* Need to figure out how to get these values properly */
        send_wmresize(CALL_L1, header->destination, 0, 0, 1920, 1080);
        send_wmshow(CALL_L1, header->destination);

    }

    else
    {

        if (viewfocus->remotefocus)
            deactivateremote(viewfocus->remotefocus);

        viewfocus->remotefocus = remotelist.head->data;
        viewfocus->remotefocus->source = header->source;

        list_move(&viewfocus->remotes, &viewfocus->remotefocus->item);
        activateremote(viewfocus->remotefocus);
        arrangeview(header->destination, viewfocus);
        showremotes(header->destination, &viewfocus->remotes);

    }

}

static void onwmunmap(struct event_header *header)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        while (views[i].remotes.count)
        {

            struct remote *remote = views[i].remotes.head->data;

            send_wmunmap(CALL_L1, remote->source);
            list_move(&remotelist, &remote->item);

        }

    }

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

    unsigned int i;

    box_setsize(&size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&body, size.x, size.y + 48, size.w, size.h - 48);
    box_setsize(&background.size, size.x, size.y, size.w, size.h);

    for (i = 0; i < VIEWS; i++)
    {

        views[i].center = body.w / 2;

        box_setsize(&views[i].panel.size, size.x + i * size.w / VIEWS, size.y, size.w / VIEWS, 48);
        box_setsize(&views[i].number.size, views[i].panel.size.x + 12, views[i].panel.size.y + 12, views[i].panel.size.w - 24, views[i].panel.size.h - 24);
        arrangeview(header->destination, &views[i]);

    }

    mouse.x = size.x + size.w / 4;
    mouse.y = size.y + size.h / 4;

}

static void onwmshow(struct event_header *header)
{

    unsigned int i;

    print_insertfill(&output, header->destination, &background, 1);

    for (i = 0; i < VIEWS; i++)
        printinsertview(header->destination, &views[i]);

    print_insertmouse(&output, header->destination, &mouse, 3);
    showremotes(header->destination, &viewfocus->remotes);

}

static void onwmhide(struct event_header *header)
{

    unsigned int i;

    print_removefill(&output, header->destination, &background);

    for (i = 0; i < VIEWS; i++)
        printremoveview(header->destination, &views[i]);

    print_removemouse(&output, header->destination, &mouse);
    hideremotes(header->destination, &viewfocus->remotes);

}

void refresh(void)
{

    char buffer[FUDGE_BSIZE];

    file_writeall(CALL_PO, buffer, ring_read(&output, buffer, FUDGE_BSIZE));

}

void main(void)
{

    unsigned int i;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    element_initfill(&background, 2);
    element_initmouse(&mouse, 0, 0);

    for (i = 0; i < VIEWS; i++)
    {

        element_initpanel(&views[i].panel, 0);
        element_inittext(&views[i].number, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_NORMAL);

        views[i].numberstring = '1' + i;
        views[i].remotefocus = 0;

    }

    for (i = 0; i < REMOTES; i++)
    {

        list_inititem(&remotes[i].item, &remotes[i]);
        element_initwindow(&remotes[i].window, 0);
        list_add(&remotelist, &remotes[i].item);

    }

    activateview(viewfocus);

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    if (!file_walk(CALL_L2, "/system/event/key"))
        return;

    if (!file_walk(CALL_L3, "/system/event/mouse"))
        return;

    file_open(CALL_PO);
    file_open(CALL_L0);
    file_open(CALL_L1);
    file_open(CALL_L2);
    file_open(CALL_L3);

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.mousemove = onmousemove;
    handlers.mousepress = onmousepress;
    handlers.wmmap = onwmmap;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    send_wmmap(CALL_L1, 0);

    while (!quit && ev_read(&handlers, CALL_L0))
        refresh();

    file_close(CALL_L3);
    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

