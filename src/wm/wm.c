#include <abi.h>
#include <fudge.h>
#include <event/base.h>
#include <event/device.h>
#include <event/wm.h>
#include <math/box.h>
#include <keymap/keymap.h>
#include <widget/widget.h>
#include "render.h"

#define REMOTES                         64
#define VIEWS                           8

static struct remote
{

    struct list_item item;
    struct widget_window window;
    unsigned int source;

} remotes[REMOTES];

static struct view
{

    struct list_item item;
    struct list remotes;
    unsigned int center;
    struct widget_panel panel;
    char numberstring;
    struct remote *currentremote;

} views[VIEWS];

static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct box size;
static struct box body;
static struct list viewlist;
static struct list remotelist;
static struct widget_fill background;
static struct widget_mouse mouse;
static struct view *currentview = &views[0];
static unsigned int padding = 4;
static unsigned int lineheight = 12;
static unsigned int steplength;

static void updateremote(struct event_header *header, struct remote *remote)
{

    widget_update(&output, &remote->window, WIDGET_Z_MIDDLE, header->destination, WIDGET_TYPE_WINDOW, sizeof (struct widget_window), remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);
    ring_write(&output, &remote->window, sizeof (struct widget_window));

}

static void updateview(struct event_header *header, struct view *view)
{

    widget_update(&output, &view->panel, WIDGET_Z_MIDDLE, header->destination, WIDGET_TYPE_PANEL, sizeof (struct widget_panel) + view->panel.length, view->panel.size.x, view->panel.size.y, view->panel.size.w, view->panel.size.h);
    ring_write(&output, &view->panel, sizeof (struct widget_panel));
    ring_write(&output, &view->numberstring, view->panel.length);

}

static void updatemouse(struct event_header *header)
{

    widget_update(&output, &mouse, WIDGET_Z_TOP, header->destination, WIDGET_TYPE_MOUSE, sizeof (struct widget_mouse), mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);
    ring_write(&output, &mouse, sizeof (struct widget_mouse));

}

static void updatebackground(struct event_header *header)
{

    widget_update(&output, &background, WIDGET_Z_BOTTOM, header->destination, WIDGET_TYPE_FILL, sizeof (struct widget_fill), background.size.x, background.size.y, background.size.w, background.size.h);
    ring_write(&output, &background, sizeof (struct widget_fill));

}

static void removeremote(struct event_header *header, struct remote *remote)
{

    widget_remove(&output, &remote->window, WIDGET_Z_MIDDLE, header->destination);

}

static void removeview(struct event_header *header, struct view *view)
{

    widget_remove(&output, &view->panel, WIDGET_Z_MIDDLE, header->destination);

}

static void removemouse(struct event_header *header)
{

    widget_remove(&output, &mouse, WIDGET_Z_TOP, header->destination);

}

static void removebackground(struct event_header *header)
{

    widget_remove(&output, &background, WIDGET_Z_BOTTOM, header->destination);

}

static void activateview(struct view *view)
{

    view->panel.active = 1;

}

static void activateremote(struct remote *remote)
{

    remote->window.active = 1;

}

static void deactivateview(struct view *view)
{

    view->panel.active = 0;

}

static void deactivateremote(struct remote *remote)
{

    remote->window.active = 0;

}

static void resizeremote(struct remote *remote, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{

    box_setsize(&remote->window.size, x, y, w, h);
    event_sendwmresize(FILE_L0, EVENT_ADDR_SELF, remote->source, remote->window.size.x + 2, remote->window.size.y + 2, remote->window.size.w - 4, remote->window.size.h - 4, padding, lineheight);

}

static void showremotes(struct event_header *header, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_sendwmshow(FILE_L0, header->destination, remote->source);
        updateremote(header, remote);

    }

}

static void hideremotes(struct event_header *header, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_sendwmhide(FILE_L0, header->destination, remote->source);
        removeremote(header, remote);

    }

}

static void flipview(struct event_header *header, struct view *view)
{

    deactivateview(currentview);
    hideremotes(header, &currentview->remotes);
    updateview(header, currentview);

    currentview = view;

    activateview(currentview);
    showremotes(header, &currentview->remotes);
    updateview(header, currentview);

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

static void setupviews(void)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        struct view *view = &views[i];

        list_inititem(&view->item, view);
        widget_initpanel(&view->panel, 0);
        list_add(&viewlist, &view->item);

        view->numberstring = '1' + i;
        view->panel.length = 1;

    }

}

static void setupremotes(void)
{

    unsigned int i;

    for (i = 0; i < REMOTES; i++)
    {

        struct remote *remote = &remotes[i];

        list_inititem(&remote->item, remote);
        widget_initwindow(&remote->window, 0);
        list_add(&remotelist, &remote->item);

    }

}

static void oninit(struct event_header *header)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    widget_initfill(&background, 2);
    widget_initmouse(&mouse, WIDGET_MOUSETYPE_DEFAULT);
    setupviews();
    setupremotes();
    activateview(currentview);
    render_init();
    render_setvideo(FILE_L5, 1024, 768, 4);
    render_setcolormap(FILE_L6);

}

static void onkill(struct event_header *header)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current; current = current->next)
        {

            struct remote *remote = current2->data;

            event_sendwmhide(FILE_L0, header->destination, remote->source);
            event_sendkill(FILE_L0, header->destination, remote->source);

        }

    }

    quit = 1;

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress *keypress = event_payload(header);
    struct view *nextview;
    struct remote *nextremote;
    unsigned int id;

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
            event_sendwmkeypress(FILE_L0, header->destination, currentview->currentremote->source, keypress->scancode);

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

        if (nextview == currentview)
            break;

        if (currentview->currentremote && keymod & KEYMOD_SHIFT)
        {

            list_move(&nextview->remotes, &currentview->remotes, &currentview->currentremote->item);

            currentview->currentremote = (currentview->remotes.tail) ? currentview->remotes.tail->data : 0;

            if (currentview->currentremote)
                activateremote(currentview->currentremote);

            arrangeview(currentview);
            arrangeview(nextview);

            if (nextview->currentremote)
                deactivateremote(nextview->currentremote);

            nextview->currentremote = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        flipview(header, nextview);

        break;

    case 0x10:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!currentview->currentremote)
            break;

        event_sendwmhide(FILE_L0, header->destination, currentview->currentremote->source);
        event_sendkill(FILE_L0, header->destination, currentview->currentremote->source);

        break;

    case 0x19:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!file_walk(FILE_CP, "/bin/wshell"))
            break;

        id = call_spawn();

        if (!id)
            break;

        event_sendinit(FILE_L0, header->destination, id);
        event_sendexit(FILE_L0, header->destination, id);

        break;

    case 0x1C:
        if (!currentview->currentremote)
            break;

        list_move(&currentview->remotes, &currentview->remotes, &currentview->currentremote->item);
        arrangeview(currentview);
        showremotes(header, &currentview->remotes);

        break;

    case 0x23:
        if (currentview->center <= 6 * steplength)
            break;

        currentview->center -= 4 * steplength;

        arrangeview(currentview);
        showremotes(header, &currentview->remotes);

        break;

    case 0x24:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.next ? currentview->currentremote->item.next->data : currentview->remotes.head;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(header, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(header, currentview->currentremote);

        break;

    case 0x25:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.prev ? currentview->currentremote->item.prev->data : currentview->remotes.tail;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(header, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(header, currentview->currentremote);

        break;

    case 0x26:
        if (currentview->center >= 26 * steplength)
            break;

        currentview->center += 4 * steplength;

        arrangeview(currentview);
        showremotes(header, &currentview->remotes);

        break;

    case 0x2C:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        event_sendwmhide(FILE_L0, header->destination, header->destination);
        event_sendkill(FILE_L0, header->destination, header->destination);

        break;

    }

}

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease *keyrelease = event_payload(header);

    keymod = keymap_modkey(keyrelease->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
            event_sendwmkeyrelease(FILE_L0, header->destination, currentview->currentremote->source, keyrelease->scancode);

        return;

    }

}

static void onmousemove(struct event_header *header)
{

    struct event_mousemove *mousemove = event_payload(header);

    mouse.size.x += mousemove->relx;
    mouse.size.y += mousemove->rely;

    if (mouse.size.x < size.x || mouse.size.x >= size.x + size.w)
        mouse.size.x = (mousemove->relx < 0) ? size.x : size.x + size.w - 1;

    if (mouse.size.y < size.y || mouse.size.y >= size.y + size.h)
        mouse.size.y = (mousemove->rely < 0) ? size.y : size.y + size.h - 1;

    updatemouse(header);

    if (currentview->currentremote)
        event_sendwmmousemove(FILE_L0, header->destination, currentview->currentremote->source, mouse.size.x, mouse.size.y);

}

static void onmousepress(struct event_header *header)
{

    struct event_mousepress *mousepress = event_payload(header);
    struct list_item *current;

    switch (mousepress->button)
    {

    case 0x01:
        for (current = viewlist.head; current; current = current->next)
        {

            struct view *view = current->data;

            if (view == currentview)
                continue;

            if (box_isinside(&view->panel.size, mouse.size.x, mouse.size.y))
            {

                flipview(header, view);

                break;

            }

        }

        for (current = currentview->remotes.head; current; current = current->next)
        {

            struct remote *remote = current->data;

            if (remote == currentview->currentremote)
                continue;

            if (box_isinside(&remote->window.size, mouse.size.x, mouse.size.y))
            {

                deactivateremote(currentview->currentremote);
                updateremote(header, currentview->currentremote);

                currentview->currentremote = remote;

                activateremote(currentview->currentremote);
                updateremote(header, currentview->currentremote);

                break;

            }

        }

        break;

    }

    if (currentview->currentremote)
        event_sendwmmousepress(FILE_L0, header->destination, currentview->currentremote->source, mousepress->button);

}

static void onmouserelease(struct event_header *header)
{

    struct event_mouserelease *mouserelease = event_payload(header);

    if (currentview->currentremote)
        event_sendwmmouserelease(FILE_L0, header->destination, currentview->currentremote->source, mouserelease->button);

}

static void onvideomode(struct event_header *header)
{

    struct event_videomode *videomode = event_payload(header);
    unsigned int factor = (videomode->h / 320);

    lineheight = 12 + factor * 4;
    padding = 4 + factor * 2;

    render_setpaint(videomode->bpp);

    switch (factor)
    {

    case 0:
        file_walk(FILE_L9, "/data/ter-112n.pcf");
        render_setfont(FILE_L9, lineheight, padding);
        render_setmouse(&mouse, 16);

        break;

    case 1:
        file_walk(FILE_L9, "/data/ter-114n.pcf");
        render_setfont(FILE_L9, lineheight, padding);
        render_setmouse(&mouse, 16);

        break;

    case 2:
        file_walk(FILE_L9, "/data/ter-116n.pcf");
        render_setfont(FILE_L9, lineheight, padding);
        render_setmouse(&mouse, 24);

        break;

    default:
        file_walk(FILE_L9, "/data/ter-118n.pcf");
        render_setfont(FILE_L9, lineheight, padding);
        render_setmouse(&mouse, 24);

        break;

    }

    event_sendwmresize(FILE_L0, header->destination, header->destination, 0, 0, videomode->w, videomode->h, padding, lineheight);
    event_sendwmshow(FILE_L0, header->destination, header->destination);

}

static void onwmmap(struct event_header *header)
{

    if (currentview->currentremote)
        deactivateremote(currentview->currentremote);

    currentview->currentremote = remotelist.head->data;
    currentview->currentremote->source = header->source;

    list_move(&currentview->remotes, currentview->currentremote->item.list, &currentview->currentremote->item);
    activateremote(currentview->currentremote);
    arrangeview(currentview);
    showremotes(header, &currentview->remotes);

}

static void onwmunmap(struct event_header *header)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current2; current2 = current2->next)
        {

            struct remote *remote = current2->data;

            if (header->source != remote->source)
                continue;

            removeremote(header, remote);
            list_move(&remotelist, remote->item.list, &remote->item);

            view->currentremote = (view->remotes.tail) ? view->remotes.tail->data : 0;

            if (view->currentremote)
                activateremote(view->currentremote);

            arrangeview(view);

            if (view == currentview)
                showremotes(header, &view->remotes);

        }

    }

}

static void onwmresize(struct event_header *header)
{

    struct event_wmresize *wmresize = event_payload(header);
    struct list_item *current;
    unsigned int i = 0;

    box_setsize(&size, wmresize->x, wmresize->y, wmresize->w, wmresize->h);
    box_setsize(&body, size.x, size.y + (wmresize->lineheight + wmresize->padding * 2), size.w, size.h - (wmresize->lineheight + wmresize->padding * 2));
    box_setsize(&background.size, size.x, size.y, size.w, size.h);

    steplength = body.w / 32;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        view->center = 18 * steplength;

        box_setsize(&view->panel.size, size.x + i * size.w / viewlist.count, size.y, size.w / viewlist.count, (wmresize->lineheight + wmresize->padding * 2));
        arrangeview(view);

        i++;

    }

    mouse.size.x = size.x + size.w / 4;
    mouse.size.y = size.y + size.h / 4;

}

static void onwmshow(struct event_header *header)
{

    struct list_item *current;

    updatebackground(header);
    updatemouse(header);

    for (current = viewlist.head; current; current = current->next)
        updateview(header, current->data);

    showremotes(header, &currentview->remotes);

}

static void onwmhide(struct event_header *header)
{

    struct list_item *current;

    removebackground(header);
    removemouse(header);

    for (current = viewlist.head; current; current = current->next)
        removeview(header, current->data);

    hideremotes(header, &currentview->remotes);

}

static void onwmflush(struct event_header *header)
{

    void *data = event_payload(header);

    render_begin(data, header->length - sizeof (struct event_header));
    render_update(FILE_L4, size.w, size.h);
    render_complete();

}

void main(void)
{

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/keyboard/event"))
        return;

    if (!file_walk(FILE_L2, "/system/mouse/event"))
        return;

    if (!file_walk(FILE_L3, "/system/video/if:0/event"))
        return;

    if (!file_walk(FILE_L4, "/system/video/if:0/data"))
        return;

    if (!file_walk(FILE_L5, "/system/video/if:0/ctrl"))
        return;

    if (!file_walk(FILE_L6, "/system/video/if:0/colormap"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    file_open(FILE_L2);
    file_open(FILE_L3);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_KEYPRESS:
            onkeypress(header);

            break;

        case EVENT_KEYRELEASE:
            onkeyrelease(header);

            break;

        case EVENT_MOUSEMOVE:
            onmousemove(header);

            break;

        case EVENT_MOUSEPRESS:
            onmousepress(header);

            break;

        case EVENT_MOUSERELEASE:
            onmouserelease(header);

            break;

        case EVENT_VIDEOMODE:
            onvideomode(header);

            break;

        case EVENT_WMMAP:
            onwmmap(header);

            break;

        case EVENT_WMUNMAP:
            onwmunmap(header);

            break;

        case EVENT_WMRESIZE:
            onwmresize(header);

            break;

        case EVENT_WMSHOW:
            onwmshow(header);

            break;

        case EVENT_WMHIDE:
            onwmhide(header);

            break;

        case EVENT_WMFLUSH:
            onwmflush(header);

            break;

        }

        if (ring_count(&output))
        {

            event_sendwmflush(FILE_L0, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, ring_count(&output), outputdata);
            ring_reset(&output);

        }

    }

    file_close(FILE_L3);
    file_close(FILE_L2);
    file_close(FILE_L1);
    file_close(FILE_L0);

}

