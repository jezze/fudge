#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>
#include <widget/render.h>

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

static unsigned int keymod = KEYMOD_NONE;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct box screen;
static struct box body;
static struct list viewlist;
static struct list remotelist;
static struct widget_fill background;
static struct widget_mouse mouse;
static struct view *currentview = &views[0];
static unsigned int padding;
static unsigned int lineheight;
static unsigned int steplength;

static void updateremote(union event_message *imessage, struct remote *remote)
{

    widget_update(&output, &remote->window, WIDGET_Z_MIDDLE, imessage->header.target, WIDGET_TYPE_WINDOW, sizeof (struct widget_window), remote->window.size.x, remote->window.size.y, remote->window.size.w, remote->window.size.h);
    ring_write(&output, &remote->window, sizeof (struct widget_window));

}

static void updateview(union event_message *imessage, struct view *view)
{

    widget_update(&output, &view->panel, WIDGET_Z_MIDDLE, imessage->header.target, WIDGET_TYPE_PANEL, sizeof (struct widget_panel) + view->panel.length, view->panel.size.x, view->panel.size.y, view->panel.size.w, view->panel.size.h);
    ring_write(&output, &view->panel, sizeof (struct widget_panel));
    ring_write(&output, &view->numberstring, view->panel.length);

}

static void updatemouse(union event_message *imessage)
{

    widget_update(&output, &mouse, WIDGET_Z_TOP, imessage->header.target, WIDGET_TYPE_MOUSE, sizeof (struct widget_mouse), mouse.size.x, mouse.size.y, mouse.size.w, mouse.size.h);
    ring_write(&output, &mouse, sizeof (struct widget_mouse));

}

static void updatebackground(union event_message *imessage)
{

    widget_update(&output, &background, WIDGET_Z_BOTTOM, imessage->header.target, WIDGET_TYPE_FILL, sizeof (struct widget_fill), background.size.x, background.size.y, background.size.w, background.size.h);
    ring_write(&output, &background, sizeof (struct widget_fill));

}

static void removeremote(union event_message *imessage, struct remote *remote)
{

    widget_remove(&output, &remote->window, WIDGET_Z_MIDDLE, imessage->header.target);

}

static void removeview(union event_message *imessage, struct view *view)
{

    widget_remove(&output, &view->panel, WIDGET_Z_MIDDLE, imessage->header.target);

}

static void removemouse(union event_message *imessage)
{

    widget_remove(&output, &mouse, WIDGET_Z_TOP, imessage->header.target);

}

static void removebackground(union event_message *imessage)
{

    widget_remove(&output, &background, WIDGET_Z_BOTTOM, imessage->header.target);

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

static void showremotes(struct event_channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(channel, EVENT_WMSHOW, 0);
        event_place(remote->source, &channel->o);
        updateremote(&channel->i, remote);

    }

}

static void hideremotes(struct event_channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(channel, EVENT_WMHIDE, 0);
        event_place(remote->source, &channel->o);
        removeremote(&channel->i, remote);

    }

}

static void configureremotes(struct event_channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(channel, EVENT_WMCONFIGURE, 0);
        event_addwmconfigure(&channel->o, remote->window.size.x + 2, remote->window.size.y + 2, remote->window.size.w - 4, remote->window.size.h - 4, padding, lineheight);
        event_place(remote->source, &channel->o);

    }

}

static void killremotes(struct event_channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(channel, EVENT_KILL, 0);
        event_place(remote->source, &channel->o);

    }

}

static void flipview(struct event_channel *channel, struct view *view)
{

    deactivateview(currentview);
    hideremotes(channel, &currentview->remotes);
    updateview(&channel->i, currentview);

    currentview = view;

    activateview(currentview);
    showremotes(channel, &currentview->remotes);
    updateview(&channel->i, currentview);

}

static void arrangesingle(struct view *view)
{

    struct remote *remote = view->remotes.tail->data;

    box_setsize(&remote->window.size, body.x, body.y, body.w, body.h);

}

static void arrangetiled(struct view *view)
{

    unsigned int y = body.y;
    unsigned int h = body.h / (view->remotes.count - 1);
    struct list_item *current;
    struct remote *remote = view->remotes.tail->data;

    box_setsize(&remote->window.size, body.x, body.y, view->center, body.h);

    for (current = view->remotes.tail->prev; current; current = current->prev)
    {

        struct remote *remote = current->data;

        box_setsize(&remote->window.size, body.x + view->center, y, body.w - view->center, h);

        y += h;

    }

}

static void arrangeview(struct event_channel *channel, struct view *view)
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

    configureremotes(channel, &view->remotes);

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

static unsigned int ondata(struct event_channel *channel)
{

    if (!file_walk(FILE_L0, FILE_G4, "../data"))
        return 0;

    render_write(event_getdata(channel), event_getdatasize(channel));
    render_flush(FILE_L0);
    render_complete();

    return 0;

}

static unsigned int oninit(struct event_channel *channel)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    widget_initfill(&background, 2);
    widget_initmouse(&mouse, WIDGET_MOUSETYPE_DEFAULT);
    setupviews();
    setupremotes();
    activateview(currentview);
    render_init();

    if (!file_walk(FILE_L0, FILE_G4, "../ctrl"))
        return 1;

    render_setvideo(FILE_L0, 1024, 768, 4);

    if (!file_walk(FILE_L0, FILE_G4, "../colormap"))
        return 1;

    render_setcolormap(FILE_L0);

    return 0;

}

static unsigned int onkill(struct event_channel *channel)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        hideremotes(channel, &view->remotes);
        killremotes(channel, &view->remotes);

    }

    return 1;

}

static unsigned int onstop(struct event_channel *channel)
{

    return 0;

}

static unsigned int onkeypress(struct event_channel *channel)
{

    struct event_keypress *keypress = event_getdata(channel);
    struct view *nextview;
    struct remote *nextremote;
    unsigned int id;

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            event_request(channel, EVENT_WMKEYPRESS, 0);
            event_addwmkeypress(&channel->o, keypress->scancode);
            event_place(currentview->currentremote->source, &channel->o);

        }

        return 0;

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

            arrangeview(channel, currentview);
            arrangeview(channel, nextview);

            if (nextview->currentremote)
                deactivateremote(nextview->currentremote);

            nextview->currentremote = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        flipview(channel, nextview);

        break;

    case 0x10:
        if (!currentview->currentremote)
            break;

        if ((keymod & KEYMOD_SHIFT))
        {

            event_request(channel, EVENT_WMHIDE, 0);
            event_place(currentview->currentremote->source, &channel->o);
            event_request(channel, EVENT_KILL, 0);
            event_place(currentview->currentremote->source, &channel->o);

        }

        break;

    case 0x19:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        if (!file_walk2(FILE_CP, "/bin/wshell"))
            break;

        id = call_spawn();

        if (id)
        {

            event_request(channel, EVENT_INIT, 0);
            event_place(id, &channel->o);
            event_request(channel, EVENT_STOP, 0);
            event_place(id, &channel->o);

        }

        break;

    case 0x1C:
        if (!currentview->currentremote)
            break;

        list_move(&currentview->remotes, &currentview->remotes, &currentview->currentremote->item);
        arrangeview(channel, currentview);
        showremotes(channel, &currentview->remotes);

        break;

    case 0x23:
        if (currentview->center <= 6 * steplength)
            break;

        currentview->center -= 4 * steplength;

        arrangeview(channel, currentview);
        showremotes(channel, &currentview->remotes);

        break;

    case 0x24:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.next ? currentview->currentremote->item.next->data : currentview->remotes.head;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(&channel->i, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(&channel->i, currentview->currentremote);

        break;

    case 0x25:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.prev ? currentview->currentremote->item.prev->data : currentview->remotes.tail;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(&channel->i, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(&channel->i, currentview->currentremote);

        break;

    case 0x26:
        if (currentview->center >= 26 * steplength)
            break;

        currentview->center += 4 * steplength;

        arrangeview(channel, currentview);
        showremotes(channel, &currentview->remotes);

        break;

    case 0x2C:
        if ((keymod & KEYMOD_SHIFT))
        {

            event_request(channel, EVENT_WMHIDE, 0);
            event_place(0, &channel->o);
            event_request(channel, EVENT_KILL, 0);
            event_place(0, &channel->o);

        }

        break;

    }

    return 0;

}

static unsigned int onkeyrelease(struct event_channel *channel)
{

    struct event_keyrelease *keyrelease = event_getdata(channel);

    keymod = keymap_modkey(keyrelease->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            event_request(channel, EVENT_WMKEYRELEASE, 0);
            event_addwmkeyrelease(&channel->o, keyrelease->scancode);
            event_place(currentview->currentremote->source, &channel->o);

        }

        return 0;

    }

    return 0;

}

static unsigned int onmousemove(struct event_channel *channel)
{

    struct event_mousemove *mousemove = event_getdata(channel);

    mouse.size.x += mousemove->relx;
    mouse.size.y += mousemove->rely;

    if (mouse.size.x < screen.x || mouse.size.x >= screen.x + screen.w)
        mouse.size.x = (mousemove->relx < 0) ? screen.x : screen.x + screen.w - 1;

    if (mouse.size.y < screen.y || mouse.size.y >= screen.y + screen.h)
        mouse.size.y = (mousemove->rely < 0) ? screen.y : screen.y + screen.h - 1;

    updatemouse(&channel->i);

    if (currentview->currentremote)
    {

        event_request(channel, EVENT_WMMOUSEMOVE, 0);
        event_addwmmousemove(&channel->o, mouse.size.x, mouse.size.y);
        event_place(currentview->currentremote->source, &channel->o);

    }

    return 0;

}

static unsigned int onmousepress(struct event_channel *channel)
{

    struct event_mousepress *mousepress = event_getdata(channel);
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

                flipview(channel, view);

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
                updateremote(&channel->i, currentview->currentremote);

                currentview->currentremote = remote;

                activateremote(currentview->currentremote);
                updateremote(&channel->i, currentview->currentremote);

                break;

            }

        }

        break;

    }

    if (currentview->currentremote)
    {

        event_request(channel, EVENT_WMMOUSEPRESS, 0);
        event_addwmmousepress(&channel->o, mousepress->button);
        event_place(currentview->currentremote->source, &channel->o);

    }

    return 0;

}

static unsigned int onmouserelease(struct event_channel *channel)
{

    struct event_mouserelease *mouserelease = event_getdata(channel);

    if (currentview->currentremote)
    {

        event_request(channel, EVENT_WMMOUSERELEASE, 0);
        event_addwmmouserelease(&channel->o, mouserelease->button);
        event_place(currentview->currentremote->source, &channel->o);

    }

    return 0;

}

static unsigned int onvideomode(struct event_channel *channel)
{

    struct event_videomode *videomode = event_getdata(channel);
    unsigned int factor = (videomode->h / 320);

    lineheight = 12 + factor * 4;
    padding = 4 + factor * 2;

    render_setdraw(videomode->w, videomode->h, videomode->bpp);

    switch (factor)
    {

    case 0:
    case 1:
        mouse.size.w = 12;
        mouse.size.h = 16;

        break;

    case 2:
    default:
        mouse.size.w = 18;
        mouse.size.h = 24;

        break;

    }

    switch (factor)
    {

    case 0:
        file_walk2(FILE_L0, "/data/ter-112n.pcf");
        render_setfont(FILE_L0, lineheight, padding);

        break;

    case 1:
        file_walk2(FILE_L0, "/data/ter-114n.pcf");
        render_setfont(FILE_L0, lineheight, padding);

        break;

    case 2:
        file_walk2(FILE_L0, "/data/ter-116n.pcf");
        render_setfont(FILE_L0, lineheight, padding);

        break;

    default:
        file_walk2(FILE_L0, "/data/ter-118n.pcf");
        render_setfont(FILE_L0, lineheight, padding);

        break;

    }

    event_request(channel, EVENT_WMCONFIGURE, 0);
    event_addwmconfigure(&channel->o, 0, 0, videomode->w, videomode->h, padding, lineheight);
    event_place(0, &channel->o);
    event_request(channel, EVENT_WMSHOW, 0);
    event_place(0, &channel->o);

    return 0;

}

static unsigned int onwmconfigure(struct event_channel *channel)
{

    struct event_wmconfigure *wmconfigure = event_getdata(channel);
    struct list_item *current;
    unsigned int i = 0;

    box_setsize(&screen, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);
    box_setsize(&body, wmconfigure->x, wmconfigure->y + (wmconfigure->lineheight + wmconfigure->padding * 2), wmconfigure->w, wmconfigure->h - (wmconfigure->lineheight + wmconfigure->padding * 2));
    box_setsize(&background.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);

    steplength = wmconfigure->w / 32;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        view->center = 18 * steplength;

        box_setsize(&view->panel.size, wmconfigure->x + i * wmconfigure->w / viewlist.count, wmconfigure->y, wmconfigure->w / viewlist.count, (wmconfigure->lineheight + wmconfigure->padding * 2));
        arrangeview(channel, view);

        i++;

    }

    mouse.size.x = wmconfigure->x + wmconfigure->w / 4;
    mouse.size.y = wmconfigure->y + wmconfigure->h / 4;

    return 0;

}

static unsigned int onwmmap(struct event_channel *channel)
{

    if (currentview->currentremote)
        deactivateremote(currentview->currentremote);

    currentview->currentremote = remotelist.head->data;
    currentview->currentremote->source = channel->i.header.source;

    list_move(&currentview->remotes, currentview->currentremote->item.list, &currentview->currentremote->item);
    activateremote(currentview->currentremote);
    arrangeview(channel, currentview);
    showremotes(channel, &currentview->remotes);

    return 0;

}

static unsigned int onwmunmap(struct event_channel *channel)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current2; current2 = current2->next)
        {

            struct remote *remote = current2->data;

            if (channel->i.header.source != remote->source)
                continue;

            removeremote(&channel->i, remote);
            list_move(&remotelist, remote->item.list, &remote->item);

            view->currentremote = (view->remotes.tail) ? view->remotes.tail->data : 0;

            if (view->currentremote)
                activateremote(view->currentremote);

            arrangeview(channel, view);

            if (view == currentview)
                showremotes(channel, &view->remotes);

        }

    }

    return 0;

}

static unsigned int onwmshow(struct event_channel *channel)
{

    struct list_item *current;

    updatebackground(&channel->i);
    updatemouse(&channel->i);

    for (current = viewlist.head; current; current = current->next)
        updateview(&channel->i, current->data);

    showremotes(channel, &currentview->remotes);

    return 0;

}

static unsigned int onwmhide(struct event_channel *channel)
{

    struct list_item *current;

    removebackground(&channel->i);
    removemouse(&channel->i);

    for (current = viewlist.head; current; current = current->next)
        removeview(&channel->i, current->data);

    hideremotes(channel, &currentview->remotes);

    return 0;

}

void init(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);
    event_setsignal(EVENT_INIT, oninit);
    event_setsignal(EVENT_KILL, onkill);
    event_setsignal(EVENT_STOP, onstop);
    event_setsignal(EVENT_KEYPRESS, onkeypress);
    event_setsignal(EVENT_KEYRELEASE, onkeyrelease);
    event_setsignal(EVENT_MOUSEMOVE, onmousemove);
    event_setsignal(EVENT_MOUSEPRESS, onmousepress);
    event_setsignal(EVENT_MOUSERELEASE, onmouserelease);
    event_setsignal(EVENT_VIDEOMODE, onvideomode);
    event_setsignal(EVENT_WMCONFIGURE, onwmconfigure);
    event_setsignal(EVENT_WMMAP, onwmmap);
    event_setsignal(EVENT_WMUNMAP, onwmunmap);
    event_setsignal(EVENT_WMSHOW, onwmshow);
    event_setsignal(EVENT_WMHIDE, onwmhide);

}

void main(void)
{

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    if (!file_walk2(FILE_G1, "/system/event"))
        return;

    if (!file_walk2(FILE_G2, "/system/keyboard/event"))
        return;

    if (!file_walk2(FILE_G3, "/system/mouse/event"))
        return;

    if (!file_walk2(FILE_G4, "/system/video/if:0/event"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    file_open(FILE_G2);
    file_open(FILE_G3);
    file_open(FILE_G4);

    while (event_listen())
    {

        if (ring_count(&output))
        {

            union event_message message;

            event_create(&message, EVENT_DATA);
            event_append(&message, ring_count(&output), outputdata);
            ring_reset(&output);
            file_writeall(FILE_G0, &message, message.header.length);

        }

    }

    file_close(FILE_G4);
    file_close(FILE_G3);
    file_close(FILE_G2);
    file_close(FILE_G1);
    file_close(FILE_G0);

}

