#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "send.h"
#include "keymap.h"

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
static struct buffer output;
static struct box size;
static struct box body;
static struct list remotelist;
static struct element_fill background;
static struct element_mouse mouse;
static struct view *viewfocus;
static void (*handlers[EVENTS])(struct event_header *header);

static void printremote(struct buffer *buffer, unsigned int source, unsigned int z, struct remote *remote)
{

    print_window(buffer, source, z, &remote->window);

}

static void printview(struct buffer *buffer, unsigned int source, unsigned int z, struct view *view)
{

    print_panel(buffer, source, z, &view->panel);
    print_text(buffer, source, z, &view->number, &view->numberstring, 1);

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

        send_wmshow(CALL_L1, remote->source);
        printremote(&output, source, 1, remote);

    }

}

static void hideremotes(unsigned int source, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        send_wmhide(CALL_L1, remote->source);
        printremote(&output, source, 0, remote);

    }

}

static void arrangeview(unsigned int source, struct view *view)
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

        box_setsize(&remote->window.size, body.x, body.y, body.w, body.h);
        send_wmresize(CALL_L1, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

        break;

    default:
        y = body.y;
        h = body.h / (view->remotes.count - 1);
        remote = current->data;

        box_setsize(&remote->window.size, body.x, body.y, view->center, body.h);
        send_wmresize(CALL_L1, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

        for (current = view->remotes.tail->prev; current; current = current->prev)
        {

            remote = current->data;

            box_setsize(&remote->window.size, body.x + view->center, y, body.w - view->center, h);
            send_wmresize(CALL_L1, remote->source, remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);

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
    printview(&output, source, 1, view);
    showremotes(source, &view->remotes);

}

static void hideview(unsigned int source, struct view *view)
{

    deactivateview(view);
    printview(&output, source, 1, view);
    hideremotes(source, &view->remotes);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;
    struct view *nextview;
    struct remote *nextremote;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    switch (keypress.scancode)
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
            send_keypress(CALL_L2, viewfocus->remotefocus->source, keypress.scancode);

        return;

    }

    switch (keypress.scancode)
    {

    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        nextview = &views[keypress.scancode - 0x02];

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
        printremote(&output, header->destination, 0, viewfocus->remotefocus);
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
        printremote(&output, header->destination, 1, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printremote(&output, header->destination, 1, viewfocus->remotefocus);

        break;

    case 0x25:
        if (!viewfocus->remotefocus)
            break;

        nextremote = viewfocus->remotefocus->item.prev ? viewfocus->remotefocus->item.prev->data : viewfocus->remotes.tail;

        if (!nextremote || nextremote == viewfocus->remotefocus)
            break;

        deactivateremote(viewfocus->remotefocus);
        printremote(&output, header->destination, 1, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printremote(&output, header->destination, 1, viewfocus->remotefocus);

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

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(CALL_L0, &keyrelease, sizeof (struct event_keyrelease));

    switch (keyrelease.scancode)
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
            send_keyrelease(CALL_L2, viewfocus->remotefocus->source, keyrelease.scancode);

        return;

    }

}

static void onmousemove(struct event_header *header)
{

    struct event_mousemove mousemove;

    file_readall(CALL_L0, &mousemove, sizeof (struct event_mousemove));

    mouse.x += mousemove.relx;
    mouse.y -= mousemove.rely;

    if (mousemove.relx > 0 && mouse.x >= size.w)
        mouse.x = size.w - 1;

    if (mousemove.relx < 0 && mouse.x >= size.w)
        mouse.x = 0;

    if (mousemove.rely < 0 && mouse.y >= size.h)
        mouse.y = size.h - 1;

    if (mousemove.rely > 0 && mouse.y >= size.h)
        mouse.y = 0;

    print_mouse(&output, header->destination, 3, &mouse);

}

static void onmousepress(struct event_header *header)
{

    struct event_mousepress mousepress;
    struct list_item *current;
    unsigned int i;

    file_readall(CALL_L0, &mousepress, sizeof (struct event_mousepress));

    switch (mousepress.button)
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
            printremote(&output, header->destination, 1, viewfocus->remotefocus);

            viewfocus->remotefocus = remote;

            activateremote(viewfocus->remotefocus);
            printremote(&output, header->destination, 1, viewfocus->remotefocus);

        }

        break;

    }

}

static void onmouserelease(struct event_header *header)
{

    struct event_mouserelease mouserelease;

    file_readall(CALL_L0, &mouserelease, sizeof (struct event_mouserelease));

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

static void onwmresize(struct event_header *header)
{

    struct event_wmresize wmresize;
    unsigned int i;

    file_readall(CALL_L0, &wmresize, sizeof (struct event_wmresize));
    box_setsize(&size, wmresize.x, wmresize.y, wmresize.w, wmresize.h);
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

    print_fill(&output, header->destination, 1, &background);

    for (i = 0; i < VIEWS; i++)
        printview(&output, header->destination, 1, &views[i]);

    print_mouse(&output, header->destination, 3, &mouse);
    showremotes(header->destination, &viewfocus->remotes);

}

static void onwmhide(struct event_header *header)
{

    unsigned int i;

    print_fill(&output, header->destination, 0, &background);

    for (i = 0; i < VIEWS; i++)
        printview(&output, header->destination, 0, &views[i]);

    print_mouse(&output, header->destination, 0, &mouse);
    hideremotes(header->destination, &viewfocus->remotes);

}

void main(void)
{

    struct event_header header;
    unsigned int count;
    unsigned int i;

    buffer_init(&output, FUDGE_BSIZE, outputdata);
    element_initfill(&background, 2);
    element_initmouse(&mouse, 0, 0);

    for (i = 0; i < REMOTES; i++)
    {

        list_inititem(&remotes[i].item, &remotes[i]);
        element_initwindow(&remotes[i].window, 0);
        list_add(&remotelist, &remotes[i].item);

    }

    for (i = 0; i < VIEWS; i++)
    {

        element_initpanel(&views[i].panel, 0);
        element_inittext(&views[i].number, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_NORMAL);

        views[i].numberstring = '1' + i;
        views[i].remotefocus = 0;

    }

    viewfocus = &views[0];
    viewfocus->panel.active = 1;
    viewfocus->number.type = ELEMENT_TEXTTYPE_HIGHLIGHT;

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

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_MOUSEMOVE] = onmousemove;
    handlers[EVENT_MOUSEPRESS] = onmousepress;
    handlers[EVENT_MOUSERELEASE] = onmouserelease;
    handlers[EVENT_WMMAP] = onwmmap;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    send_wmmap(CALL_L1, 0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        if (!handlers[header.type])
            continue;

        handlers[header.type](&header);

        if (output.count)
        {

            file_writeall(CALL_PO, output.memory, output.count);
            buffer_init(&output, FUDGE_BSIZE, outputdata);

        }

        if (quit)
            break;

    }

    file_close(CALL_L3);
    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

