#include <fudge.h>
#include <abi.h>
#include <widget.h>

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

static void updateremote(struct remote *remote)
{

    widget_update(&output, &remote->window, WIDGET_Z_MIDDLE, WIDGET_TYPE_WINDOW, sizeof (struct widget_window), &remote->window.size);
    ring_write(&output, &remote->window, sizeof (struct widget_window));

}

static void updateview(struct view *view)
{

    widget_update(&output, &view->panel, WIDGET_Z_MIDDLE, WIDGET_TYPE_PANEL, sizeof (struct widget_panel) + view->panel.length, &view->panel.size);
    ring_write(&output, &view->panel, sizeof (struct widget_panel));
    ring_write(&output, &view->numberstring, view->panel.length);

}

static void updatemouse(void)
{

    widget_update(&output, &mouse, WIDGET_Z_TOP, WIDGET_TYPE_MOUSE, sizeof (struct widget_mouse), &mouse.size);
    ring_write(&output, &mouse, sizeof (struct widget_mouse));

}

static void updatebackground(void)
{

    widget_update(&output, &background, WIDGET_Z_BOTTOM, WIDGET_TYPE_FILL, sizeof (struct widget_fill), &background.size);
    ring_write(&output, &background, sizeof (struct widget_fill));

}

static void removeremote(struct remote *remote)
{

    widget_remove(&output, &remote->window, WIDGET_Z_MIDDLE);

}

static void removeview(struct view *view)
{

    widget_remove(&output, &view->panel, WIDGET_Z_MIDDLE);

}

static void removemouse(void)
{

    widget_remove(&output, &mouse, WIDGET_Z_TOP);

}

static void removebackground(void)
{

    widget_remove(&output, &background, WIDGET_Z_BOTTOM);

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

static void showremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        channel_request(channel, EVENT_WMSHOW);
        channel_place(channel, remote->source);
        updateremote(remote);

    }

}

static void hideremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        channel_request(channel, EVENT_WMHIDE);
        channel_place(channel, remote->source);
        removeremote(remote);

    }

}

static void configureremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;
        struct event_wmconfigure wmconfigure;

        wmconfigure.x = remote->window.size.x + 2;
        wmconfigure.y = remote->window.size.y + 2;
        wmconfigure.w = remote->window.size.w - 4;
        wmconfigure.h = remote->window.size.h - 4;
        wmconfigure.padding = padding;
        wmconfigure.lineheight = lineheight;

        channel_request(channel, EVENT_WMCONFIGURE);
        channel_append(channel, sizeof (struct event_wmconfigure), &wmconfigure);
        channel_place(channel, remote->source);

    }

}

static void closeremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        channel_request(channel, EVENT_WMCLOSE);
        channel_place(channel, remote->source);

    }

}

static void flipview(struct channel *channel, struct view *view)
{

    deactivateview(currentview);
    hideremotes(channel, &currentview->remotes);
    updateview(currentview);

    currentview = view;

    activateview(currentview);
    showremotes(channel, &currentview->remotes);
    updateview(currentview);

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

static void arrangeview(struct channel *channel, struct view *view)
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

static void setupvideo(void)
{

    render_init();

    if (!file_walk(FILE_L0, FILE_P0, "ctrl"))
        return;

    render_setvideo(FILE_L0, 1024, 768, 4);

    if (!file_walk(FILE_L0, FILE_P0, "colormap"))
        return;

    render_setcolormap(FILE_L0);

}

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    render_write(channel->i.source, mdata, msize);
    render_flush(FILE_G5);
    render_complete();

}

static void onkeypress(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    struct view *nextview;
    struct remote *nextremote;
    unsigned int id;

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            struct event_wmkeypress wmkeypress;

            wmkeypress.scancode = keypress->scancode;

            channel_request(channel, EVENT_WMKEYPRESS);
            channel_append(channel, sizeof (struct event_wmkeypress), &wmkeypress);
            channel_place(channel, currentview->currentremote->source);

        }

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

            channel_request(channel, EVENT_WMHIDE);
            channel_place(channel, currentview->currentremote->source);
            channel_request(channel, EVENT_WMCLOSE);
            channel_place(channel, currentview->currentremote->source);

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

            channel_request(channel, EVENT_OPEN);
            channel_place(channel, id);
            channel_request(channel, EVENT_CLOSE);
            channel_place(channel, id);

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
        updateremote(currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(currentview->currentremote);

        break;

    case 0x25:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.prev ? currentview->currentremote->item.prev->data : currentview->remotes.tail;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(currentview->currentremote);

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

            channel_request(channel, EVENT_WMHIDE);
            channel_place(channel, 0);
            channel_request(channel, EVENT_WMCLOSE);
            channel_place(channel, 0);

        }

        break;

    }

}

static void onkeyrelease(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            struct event_wmkeyrelease wmkeyrelease;

            wmkeyrelease.scancode = keyrelease->scancode;

            channel_request(channel, EVENT_WMKEYRELEASE);
            channel_append(channel, sizeof (struct event_wmkeyrelease), &wmkeyrelease);
            channel_place(channel, currentview->currentremote->source);

        }

        return;

    }

}

static void onmousemove(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_mousemove *mousemove = mdata;

    mouse.size.x += mousemove->relx;
    mouse.size.y += mousemove->rely;

    if (mouse.size.x < screen.x || mouse.size.x >= screen.x + screen.w)
        mouse.size.x = (mousemove->relx < 0) ? screen.x : screen.x + screen.w - 1;

    if (mouse.size.y < screen.y || mouse.size.y >= screen.y + screen.h)
        mouse.size.y = (mousemove->rely < 0) ? screen.y : screen.y + screen.h - 1;

    updatemouse();

    if (currentview->currentremote)
    {

        struct event_wmmousemove wmmousemove;

        wmmousemove.relx = mousemove->relx;
        wmmousemove.rely = mousemove->rely;

        channel_request(channel, EVENT_WMMOUSEMOVE);
        channel_append(channel, sizeof (struct event_wmmousemove), &wmmousemove);
        channel_place(channel, currentview->currentremote->source);

    }

}

static void onmousepress(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_mousepress *mousepress = mdata;
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
                updateremote(currentview->currentremote);

                currentview->currentremote = remote;

                activateremote(currentview->currentremote);
                updateremote(currentview->currentremote);

                break;

            }

        }

        break;

    }

    if (currentview->currentremote)
    {

        struct event_wmmousepress wmmousepress;

        wmmousepress.button = mousepress->button;

        channel_request(channel, EVENT_WMMOUSEPRESS);
        channel_append(channel, sizeof (struct event_wmmousepress), &wmmousepress);
        channel_place(channel, currentview->currentremote->source);

    }

}

static void onmouserelease(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

    if (currentview->currentremote)
    {

        struct event_wmmouserelease wmmouserelease;

        wmmouserelease.button = mouserelease->button;

        channel_request(channel, EVENT_WMMOUSERELEASE);
        channel_append(channel, sizeof (struct event_wmmouserelease), &wmmouserelease);
        channel_place(channel, currentview->currentremote->source);

    }

}

static void onvideomode(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = (videomode->h / 320);
    struct event_wmconfigure wmconfigure;

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

    wmconfigure.x = 0;
    wmconfigure.y = 0;
    wmconfigure.w = videomode->w;
    wmconfigure.h = videomode->h;
    wmconfigure.padding = padding;
    wmconfigure.lineheight = lineheight;

    channel_request(channel, EVENT_WMCONFIGURE);
    channel_append(channel, sizeof (struct event_wmconfigure), &wmconfigure);
    channel_place(channel, 0);
    channel_request(channel, EVENT_WMSHOW);
    channel_place(channel, 0);

}

static void onwmconfigure(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_wmconfigure *wmconfigure = mdata;
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

}

static void onwmmap(struct channel *channel, void *mdata, unsigned int msize)
{

    if (currentview->currentremote)
        deactivateremote(currentview->currentremote);

    currentview->currentremote = remotelist.head->data;
    currentview->currentremote->source = channel->i.source;

    list_move(&currentview->remotes, currentview->currentremote->item.list, &currentview->currentremote->item);
    activateremote(currentview->currentremote);
    arrangeview(channel, currentview);
    showremotes(channel, &currentview->remotes);

}

static void onwmunmap(struct channel *channel, void *mdata, unsigned int msize)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current2; current2 = current2->next)
        {

            struct remote *remote = current2->data;

            if (channel->i.source != remote->source)
                continue;

            removeremote(remote);
            list_move(&remotelist, remote->item.list, &remote->item);

            view->currentremote = (view->remotes.tail) ? view->remotes.tail->data : 0;

            if (view->currentremote)
                activateremote(view->currentremote);

            arrangeview(channel, view);

            if (view == currentview)
                showremotes(channel, &view->remotes);

        }

    }

}

static void onwmshow(struct channel *channel, void *mdata, unsigned int msize)
{

    struct list_item *current;

    updatebackground();
    updatemouse();

    for (current = viewlist.head; current; current = current->next)
        updateview(current->data);

    showremotes(channel, &currentview->remotes);

}

static void onwmhide(struct channel *channel, void *mdata, unsigned int msize)
{

    struct list_item *current;

    removebackground();
    removemouse();

    for (current = viewlist.head; current; current = current->next)
        removeview(current->data);

    hideremotes(channel, &currentview->remotes);

}

static void onany(struct channel *channel, void *mdata, unsigned int msize)
{

    if (ring_count(&output))
    {

        channel_request(channel, EVENT_DATA);
        channel_append(channel, ring_count(&output), outputdata);
        channel_write(channel, FILE_G0);
        ring_reset(&output);

    }

}

void main(void)
{

    struct channel channel;
    struct list_item *current;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_ANY, onany);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_KEYPRESS, onkeypress);
    channel_setsignal(&channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_setsignal(&channel, EVENT_MOUSEMOVE, onmousemove);
    channel_setsignal(&channel, EVENT_MOUSEPRESS, onmousepress);
    channel_setsignal(&channel, EVENT_MOUSERELEASE, onmouserelease);
    channel_setsignal(&channel, EVENT_VIDEOMODE, onvideomode);
    channel_setsignal(&channel, EVENT_WMCONFIGURE, onwmconfigure);
    channel_setsignal(&channel, EVENT_WMMAP, onwmmap);
    channel_setsignal(&channel, EVENT_WMUNMAP, onwmunmap);
    channel_setsignal(&channel, EVENT_WMSHOW, onwmshow);
    channel_setsignal(&channel, EVENT_WMHIDE, onwmhide);

    if (!file_walk2(FILE_G0, "/system/multicast"))
        return;

    if (!file_walk2(FILE_G1, "/system/event"))
        return;

    if (!file_walk2(FILE_G2, "/system/keyboard/event"))
        return;

    if (!file_walk2(FILE_G3, "/system/mouse/event"))
        return;

    if (!file_walk(FILE_G4, FILE_P0, "event"))
        return;

    if (!file_walk(FILE_G5, FILE_P0, "data"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    file_open(FILE_G2);
    file_open(FILE_G3);
    file_open(FILE_G4);
    file_open(FILE_G5);
    ring_init(&output, FUDGE_BSIZE, outputdata);
    widget_initfill(&background, 2);
    widget_initmouse(&mouse, WIDGET_MOUSETYPE_DEFAULT);
    setupvideo();
    setupviews();
    setupremotes();
    activateview(currentview);
    channel_listen(&channel);

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        hideremotes(&channel, &view->remotes);
        closeremotes(&channel, &view->remotes);

    }

    file_close(FILE_G5);
    file_close(FILE_G4);
    file_close(FILE_G3);
    file_close(FILE_G2);
    file_close(FILE_G1);
    file_close(FILE_G0);

}

