#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "print.h"
#include "keymap.h"
#include "ev.h"
#include "render.h"

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
static unsigned int padding;
static unsigned int lineheight;
static unsigned int steplength;

static void printinsertremote(struct event_header *header, struct remote *remote)
{

    print_insertwindow(&output, header, &remote->window);

}

static void printremoveremote(struct event_header *header, struct remote *remote)
{

    print_removewindow(&output, header, &remote->window);

}

static void printinsertview(struct event_header *header, struct view *view)
{

    print_insertpanel(&output, header, &view->panel);
    print_inserttext(&output, header, &view->number, 1);
    ring_write(&output, &view->numberstring, 1);

}

static void printremoveview(struct event_header *header, struct view *view)
{

    print_removepanel(&output, header, &view->panel);
    print_removetext(&output, header, &view->number);

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

    box_setsize(&remote->window.element.size, x, y, w, h);
    ev_sendwmresize(CALL_L1, remote->source, remote->window.element.size.x + 2, remote->window.element.size.y + 2, remote->window.element.size.w - 4, remote->window.element.size.h - 4, padding, lineheight);

}

static void showremotes(struct event_header *header, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        ev_sendwmshow(CALL_L1, remote->source);
        printinsertremote(header, remote);

    }

}

static void hideremotes(struct event_header *header, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        ev_sendwmhide(CALL_L1, remote->source);
        printremoveremote(header, remote);

    }

}

static void arrangesingle(struct view *view)
{

    resizeremote(view->remotes.tail->data, body.x, body.y, body.w, body.h);

}

static void arrangetiled(struct view *view)
{

    unsigned int y = body.y;
    unsigned int h = body.h / (view->remotes.count - 1);
    struct list_item *current;

    resizeremote(view->remotes.tail->data, body.x, body.y, view->center, body.h);

    for (current = view->remotes.tail->prev; current; current = current->prev)
    {

        resizeremote(current->data, body.x + view->center, y, body.w - view->center, h);

        y += h;

    }

}

static void arrangeview(struct view *view)
{

    switch (view->remotes.count)
    {

    case 0:
        break;

    case 1:
        arrangesingle(view);

        break;

    default:
        arrangetiled(view);

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

static void showview(struct event_header *header, struct view *view)
{

    activateview(view);
    printinsertview(header, view);
    showremotes(header, &view->remotes);

}

static void hideview(struct event_header *header, struct view *view)
{

    deactivateview(view);
    printinsertview(header, view);
    hideremotes(header, &view->remotes);

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
            ev_sendkeypress(CALL_L1, viewfocus->remotefocus->source, keypress->scancode);

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

        hideview(header, viewfocus);

        if (viewfocus->remotefocus && keymod & KEYMOD_SHIFT)
        {

            list_move(&nextview->remotes, &viewfocus->remotefocus->item);

            viewfocus->remotefocus = (viewfocus->remotes.tail) ? viewfocus->remotes.tail->data : 0;

            if (viewfocus->remotefocus)
                activateremote(viewfocus->remotefocus);

            arrangeview(viewfocus);
            arrangeview(nextview);

            if (nextview->remotefocus)
                deactivateremote(nextview->remotefocus);

            nextview->remotefocus = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        viewfocus = nextview;

        showview(header, viewfocus);

        break;

    case 0x10:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!viewfocus->remotefocus)
            break;

        ev_sendwmhide(CALL_L1, viewfocus->remotefocus->source);
        ev_sendwmunmap(CALL_L1, viewfocus->remotefocus->source);
        printremoveremote(header, viewfocus->remotefocus);
        list_move(&remotelist, &viewfocus->remotefocus->item);

        viewfocus->remotefocus = (viewfocus->remotes.tail) ? viewfocus->remotes.tail->data : 0;

        if (viewfocus->remotefocus)
            activateremote(viewfocus->remotefocus);

        arrangeview(viewfocus);
        showremotes(header, &viewfocus->remotes);

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
        arrangeview(viewfocus);
        showremotes(header, &viewfocus->remotes);

        break;

    case 0x23:
        if (viewfocus->center <= 6 * steplength)
            break;

        viewfocus->center -= 4 * steplength;

        arrangeview(viewfocus);
        showremotes(header, &viewfocus->remotes);

        break;

    case 0x24:
        if (!viewfocus->remotefocus)
            break;

        nextremote = viewfocus->remotefocus->item.next ? viewfocus->remotefocus->item.next->data : viewfocus->remotes.head;

        if (!nextremote || nextremote == viewfocus->remotefocus)
            break;

        deactivateremote(viewfocus->remotefocus);
        printinsertremote(header, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printinsertremote(header, viewfocus->remotefocus);

        break;

    case 0x25:
        if (!viewfocus->remotefocus)
            break;

        nextremote = viewfocus->remotefocus->item.prev ? viewfocus->remotefocus->item.prev->data : viewfocus->remotes.tail;

        if (!nextremote || nextremote == viewfocus->remotefocus)
            break;

        deactivateremote(viewfocus->remotefocus);
        printinsertremote(header, viewfocus->remotefocus);

        viewfocus->remotefocus = nextremote;

        activateremote(viewfocus->remotefocus);
        printinsertremote(header, viewfocus->remotefocus);

        break;

    case 0x26:
        if (viewfocus->center >= 26 * steplength)
            break;

        viewfocus->center += 4 * steplength;

        arrangeview(viewfocus);
        showremotes(header, &viewfocus->remotes);

        break;

    case 0x2C:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        ev_sendwmhide(CALL_L1, header->destination);
        ev_sendwmunmap(CALL_L1, header->destination);

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
            ev_sendkeyrelease(CALL_L1, viewfocus->remotefocus->source, keyrelease->scancode);

        return;

    }

}

static void onmousemove(struct event_header *header, struct event_mousemove *mousemove)
{

    mouse.element.size.x += mousemove->relx;
    mouse.element.size.y += mousemove->rely;

    if (mouse.element.size.x < size.x || mouse.element.size.x >= size.x + size.w)
        mouse.element.size.x = (mousemove->relx < 0) ? size.x : size.x + size.w - 1;

    if (mouse.element.size.y < size.y || mouse.element.size.y >= size.y + size.h)
        mouse.element.size.y = (mousemove->rely < 0) ? size.y : size.y + size.h - 1;

    print_insertmouse(&output, header, &mouse);

    if (viewfocus->remotefocus)
        ev_sendmousemove(CALL_L1, viewfocus->remotefocus->source, mouse.element.size.x, mouse.element.size.y);

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

            if (!box_isinside(&views[i].panel.element.size, mouse.element.size.x, mouse.element.size.y))
                continue;

            if (&views[i] == viewfocus)
                break;

            hideview(header, viewfocus);

            viewfocus = &views[i];

            showview(header, viewfocus);

        }

        for (current = viewfocus->remotes.head; current; current = current->next)
        {

            struct remote *remote = current->data;

            if (!box_isinside(&remote->window.element.size, mouse.element.size.x, mouse.element.size.y))
                continue;

            if (remote == viewfocus->remotefocus)
                break;

            deactivateremote(viewfocus->remotefocus);
            printinsertremote(header, viewfocus->remotefocus);

            viewfocus->remotefocus = remote;

            activateremote(viewfocus->remotefocus);
            printinsertremote(header, viewfocus->remotefocus);

        }

        break;

    }

    if (viewfocus->remotefocus)
        ev_sendmousepress(CALL_L1, viewfocus->remotefocus->source, mousepress->button);

}

static void onmouserelease(struct event_header *header, struct event_mouserelease *mouserelease)
{

    if (viewfocus->remotefocus)
        ev_sendmouserelease(CALL_L1, viewfocus->remotefocus->source, mouserelease->button);

}

static void onvideomode(struct event_header *header, struct event_videomode *videomode)
{

    unsigned int factor = (videomode->h / 320);

    render_setpaint(videomode->bpp);

    switch (factor)
    {

    case 0:
        lineheight = 12 + factor * 2;
        padding = 4 + factor * 2;

        file_walk(CALL_L8, "/share/ter-112n.pcf");
        render_setfont(CALL_L8, lineheight);
        render_setmouse(&mouse, 16);

        break;

    case 1:
        lineheight = 14 + factor * 2;
        padding = 4 + factor * 2;

        file_walk(CALL_L8, "/share/ter-114n.pcf");
        render_setfont(CALL_L8, lineheight);
        render_setmouse(&mouse, 16);

        break;

    case 2:
        lineheight = 16 + factor * 2;
        padding = 4 + factor * 2;

        file_walk(CALL_L8, "/share/ter-116n.pcf");
        render_setfont(CALL_L8, lineheight);
        render_setmouse(&mouse, 24);

        break;

    default:
        lineheight = 18 + factor * 2;
        padding = 4 + factor * 2;

        file_walk(CALL_L8, "/share/ter-118n.pcf");
        render_setfont(CALL_L8, lineheight);
        render_setmouse(&mouse, 24);

        break;

    }

    ev_sendwmresize(CALL_L1, header->destination, 0, 0, videomode->w, videomode->h, padding, lineheight);
    ev_sendwmshow(CALL_L1, header->destination);

}

static void onwmmap(struct event_header *header)
{

    if (viewfocus->remotefocus)
        deactivateremote(viewfocus->remotefocus);

    viewfocus->remotefocus = remotelist.head->data;
    viewfocus->remotefocus->source = header->source;

    list_move(&viewfocus->remotes, &viewfocus->remotefocus->item);
    activateremote(viewfocus->remotefocus);
    arrangeview(viewfocus);
    showremotes(header, &viewfocus->remotes);

}

static void onwmunmap(struct event_header *header)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        while (views[i].remotes.count)
        {

            struct remote *remote = views[i].remotes.head->data;

            ev_sendwmunmap(CALL_L1, remote->source);
            list_move(&remotelist, &remote->item);

        }

    }

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

    unsigned int i;

    box_setsize(&size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&body, size.x, size.y + (wmresize->lineheight + wmresize->padding * 2), size.w, size.h - (wmresize->lineheight + wmresize->padding * 2));
    box_setsize(&background.element.size, size.x, size.y, size.w, size.h);

    steplength = body.w / 32;

    for (i = 0; i < VIEWS; i++)
    {

        views[i].center = 18 * steplength;

        box_setsize(&views[i].panel.element.size, size.x + i * size.w / VIEWS, size.y, size.w / VIEWS, (wmresize->lineheight + wmresize->padding * 2));
        box_setsize(&views[i].number.element.size, views[i].panel.element.size.x, views[i].panel.element.size.y, views[i].panel.element.size.w, views[i].panel.element.size.h);
        box_resize(&views[i].number.element.size, wmresize->padding);
        arrangeview(&views[i]);

    }

    mouse.element.size.x = size.x + size.w / 4;
    mouse.element.size.y = size.y + size.h / 4;

}

static void onwmshow(struct event_header *header)
{

    unsigned int i;

    print_insertfill(&output, header, &background);

    for (i = 0; i < VIEWS; i++)
        printinsertview(header, &views[i]);

    print_insertmouse(&output, header, &mouse);
    showremotes(header, &viewfocus->remotes);

}

static void onwmhide(struct event_header *header)
{

    unsigned int i;

    print_removefill(&output, header, &background);

    for (i = 0; i < VIEWS; i++)
        printremoveview(header, &views[i]);

    print_removemouse(&output, header, &mouse);
    hideremotes(header, &viewfocus->remotes);

}

static void onwmflush(struct event_header *header)
{

    render_parse(CALL_L0);
    render_update(CALL_L5, size.w, size.h);
    render_complete();

}

static void setup(void)
{

    unsigned int i;

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

}

void main(void)
{

    handlers.keypress = onkeypress;
    handlers.keyrelease = onkeyrelease;
    handlers.mousemove = onmousemove;
    handlers.mousepress = onmousepress;
    handlers.mouserelease = onmouserelease;
    handlers.videomode = onvideomode;
    handlers.wmmap = onwmmap;
    handlers.wmunmap = onwmunmap;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;
    handlers.wmflush = onwmflush;

    ring_init(&output, FUDGE_BSIZE, outputdata);
    element_initfill(&background, 2);
    element_initmouse(&mouse, ELEMENT_MOUSETYPE_DEFAULT);
    setup();

    if (!file_walk(CALL_L0, "/system/wm/data"))
        return;

    if (!file_walk(CALL_L1, "/system/wm/event"))
        return;

    if (!file_walk(CALL_L2, "/system/keyboard/if:0/event"))
        return;

    if (!file_walk(CALL_L3, "/system/mouse/if:0/event"))
        return;

    if (!file_walk(CALL_L4, "/system/video/if:0/event"))
        return;

    if (!file_walk(CALL_L5, "/system/video/if:0/data"))
        return;

    if (!file_walk(CALL_L6, "/system/video/if:0/ctrl"))
        return;

    if (!file_walk(CALL_L7, "/system/video/if:0/colormap"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);
    file_open(CALL_L2);
    file_open(CALL_L3);
    file_open(CALL_L4);
    render_init();
    render_setvideo(CALL_L6, 1024, 768, 32);
    render_setcolormap(CALL_L7);

    while (!quit && ev_read(&handlers, CALL_L1))
    {

        char buffer[FUDGE_BSIZE];

        if (!ring_count(&output))
            continue;

        if (file_writeall(CALL_L0, buffer, ring_read(&output, buffer, FUDGE_BSIZE)))
            ev_sendwmflush(CALL_L1, EVENT_ADDR_BROADCAST);

    }

    file_close(CALL_L4);
    file_close(CALL_L3);
    file_close(CALL_L2);
    file_close(CALL_L1);
    file_close(CALL_L0);

}

