#include <fudge.h>
#include <abi.h>
#include <widget.h>

#define REMOTES                         64
#define VIEWS                           6

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
static char outputdata[BUFFER_SIZE];
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
static unsigned char fontdata[0x8000];
static unsigned char canvasdata[0x10000];
struct event_videomode vmode;
static unsigned char colormap8[] = {
    0x00, 0x00, 0x00,
    0x3F, 0x3F, 0x3F,
    0x04, 0x02, 0x02,
    0x06, 0x04, 0x04,
    0x08, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28,
    0x1C, 0x18, 0x18,
    0x3F, 0x3F, 0x3F
};

static void draw(void *data, unsigned int count, unsigned int offset)
{

    if (vmode.fb)
        buffer_write((void *)vmode.fb, vmode.w * vmode.h * vmode.bpp, data, count, offset);
    else
        file_seekwriteall(FILE_G6, data, count, offset);

}

static void updateremote(struct remote *remote)
{

    widget_update(&output, &remote->window, WIDGET_Z_BOTTOM, WIDGET_TYPE_WINDOW, sizeof (struct widget_window), &remote->window.size);
    ring_write(&output, &remote->window, sizeof (struct widget_window));

}

static void updateview(struct view *view)
{

    widget_update(&output, &view->panel, WIDGET_Z_BOTTOM, WIDGET_TYPE_PANEL, sizeof (struct widget_panel) + view->panel.length, &view->panel.size);
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

    widget_remove(&output, &remote->window, WIDGET_Z_BOTTOM);
    render_clean(remote->source);

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

        channel_placefor(channel, remote->source, EVENT_WMSHOW, 0, 0);
        updateremote(remote);

    }

}

static void hideremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        channel_placefor(channel, remote->source, EVENT_WMHIDE, 0, 0);
        removeremote(remote);

    }

}

static void closeremotes(struct channel *channel, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        channel_placefor(channel, remote->source, EVENT_TERM, 0, 0);

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
    struct remote *master = view->remotes.tail->data;

    box_setsize(&master->window.size, body.x, body.y, view->center, body.h);

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

}

static void setupviews(void)
{

    unsigned int i;

    for (i = 0; i < VIEWS; i++)
    {

        struct view *view = &views[i];

        list_init(&view->remotes);
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

    struct ctrl_videosettings settings;
    unsigned char black[768];

    ctrl_setvideosettings(&settings, 1024, 768, 4);
    buffer_clear(black, 768);

    if (!file_walk(FILE_L0, FILE_G4, "ctrl"))
        return;

    if (!file_walk(FILE_L1, FILE_G4, "colormap"))
        return;

    file_open(FILE_L1);
    file_writeall(FILE_L1, black, 768);
    file_close(FILE_L1);
    file_open(FILE_L0);
    file_writeall(FILE_L0, &settings, sizeof (struct ctrl_videosettings));
    file_close(FILE_L0);
    file_open(FILE_L1);
    file_writeall(FILE_L1, colormap8, 3 * 11);
    file_close(FILE_L1);

}

static void onkeypress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);
    struct remote *nextremote;
    struct view *nextview;
    unsigned int id;

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            struct event_wmkeypress wmkeypress;

            wmkeypress.scancode = keypress->scancode;
            wmkeypress.unicode = keycode->value[0];
            wmkeypress.length = keycode->length;
            wmkeypress.keymod = keymod;

            channel_placefor(channel, currentview->currentremote->source, EVENT_WMKEYPRESS, sizeof (struct event_wmkeypress), &wmkeypress);

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
            channel_placefor(channel, currentview->currentremote->source, EVENT_TERM, 0, 0);

        break;

    case 0x19:
        if (!(keymod & KEYMOD_SHIFT))
            break;

        id = file_spawn("/bin/wshell");

        if (id)
            channel_placefor(channel, id, EVENT_MAIN, 0, 0);

        break;

    case 0x1C:
        if (!currentview->currentremote)
            break;

        list_move(&currentview->remotes, &currentview->remotes, &currentview->currentremote->item);
        arrangeview(channel, currentview);
        showremotes(channel, &currentview->remotes);

        break;

    case 0x23:
        if (currentview->center <= steplength)
            break;

        currentview->center -= steplength;

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
        if (currentview->center >= steplength * 11)
            break;

        currentview->center += steplength;

        arrangeview(channel, currentview);
        showremotes(channel, &currentview->remotes);

        break;

    case 0x2C:
        if ((keymod & KEYMOD_SHIFT))
        {

            struct list_item *current;

            for (current = viewlist.head; current; current = current->next)
            {

                struct view *view = current->data;

                closeremotes(channel, &view->remotes);

            }

            channel_close(channel);

        }

        break;

    }

}

static void onkeyrelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keyrelease->scancode, keymod);

    keymod = keymap_modkey(keyrelease->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            struct event_wmkeyrelease wmkeyrelease;

            wmkeyrelease.scancode = keyrelease->scancode;
            wmkeyrelease.unicode = keycode->value[0];
            wmkeyrelease.length = keycode->length;
            wmkeyrelease.keymod = keymod;

            channel_placefor(channel, currentview->currentremote->source, EVENT_WMKEYRELEASE, sizeof (struct event_wmkeyrelease), &wmkeyrelease);

        }

        return;

    }

}

static void onmousemove(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

        channel_placefor(channel, currentview->currentremote->source, EVENT_WMMOUSEMOVE, sizeof (struct event_wmmousemove), &wmmousemove);

    }

}

static void onmousescroll(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mousescroll *mousescroll = mdata;

    if (currentview->currentremote)
    {

        struct event_wmmousescroll wmmousescroll;

        wmmousescroll.relz = mousescroll->relz;

        channel_placefor(channel, currentview->currentremote->source, EVENT_WMMOUSESCROLL, sizeof (struct event_wmmousescroll), &wmmousescroll);

    }

    /* REMOVE STUFF BELOW AFTER TEST */

    if (mousescroll->relz < 0)
        currentview->center -= (currentview->center > steplength) ? steplength : 0;
    else
        currentview->center += (currentview->center < steplength) ? steplength : 0;

    arrangeview(channel, currentview);
    showremotes(channel, &currentview->remotes);

}

static void onmousepress(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
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

        channel_placefor(channel, currentview->currentremote->source, EVENT_WMMOUSEPRESS, sizeof (struct event_wmmousepress), &wmmousepress);

    }

}

static void onmouserelease(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_mouserelease *mouserelease = mdata;

    if (currentview->currentremote)
    {

        struct event_wmmouserelease wmmouserelease;

        wmmouserelease.button = mouserelease->button;

        channel_placefor(channel, currentview->currentremote->source, EVENT_WMMOUSERELEASE, sizeof (struct event_wmmouserelease), &wmmouserelease);

    }

}

static void setmouse(unsigned int x, unsigned int y, unsigned int factor)
{

    switch (factor)
    {

    case 0:
    case 1:
        box_setsize(&mouse.size, x, y, 12, 16);

        break;

    case 2:
    default:
        box_setsize(&mouse.size, x, y, 18, 24);

        break;

    }

}

static void loadfont(unsigned int factor)
{

    switch (factor)
    {

    case 0:
        file_walk2(FILE_L0, "/data/ter-112n.pcf");

        break;

    case 1:
        file_walk2(FILE_L0, "/data/ter-114n.pcf");

        break;

    case 2:
        file_walk2(FILE_L0, "/data/ter-116n.pcf");

        break;

    default:
        file_walk2(FILE_L0, "/data/ter-118n.pcf");

        break;

    }

    file_open(FILE_L0);
    file_read(FILE_L0, fontdata, 0x8000);
    file_close(FILE_L0);

}

static void onvideomode(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_videomode *videomode = mdata;
    unsigned int factor = videomode->h / 320;
    struct list_item *current;
    unsigned int i = 0;

    vmode.fb = videomode->fb;
    vmode.w = videomode->w;
    vmode.h = videomode->h;
    vmode.bpp = videomode->bpp;

    lineheight = 12 + factor * 4;
    padding = 4 + factor * 2;
    steplength = videomode->w / 12;

    loadfont(factor);
    box_setsize(&screen, 0, 0, videomode->w, videomode->h);
    box_setsize(&body, 0, (lineheight + padding * 2), videomode->w, videomode->h - (lineheight + padding * 2));
    box_setsize(&background.size, 0, 0, videomode->w, videomode->h);
    setmouse(videomode->w / 4, videomode->h / 4, factor);
    render_setdraw(videomode->w, videomode->h, videomode->bpp);
    render_setfont(fontdata, lineheight, padding);

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        view->center = steplength * 7;

        box_setsize(&view->panel.size, i * videomode->w / viewlist.count, 0, videomode->w / viewlist.count, (lineheight + padding * 2));
        arrangeview(channel, view);

        i++;

    }

    updatebackground();
    updatemouse();

    for (current = viewlist.head; current; current = current->next)
        updateview(current->data);

    showremotes(channel, &currentview->remotes);

}

static void onwmmap(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (currentview->currentremote)
        deactivateremote(currentview->currentremote);

    currentview->currentremote = remotelist.head->data;
    currentview->currentremote->source = source;

    list_move(&currentview->remotes, &remotelist, &currentview->currentremote->item);
    activateremote(currentview->currentremote);
    arrangeview(channel, currentview);
    showremotes(channel, &currentview->remotes);

}

static void onwmunmap(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current2; current2 = current2->next)
        {

            struct remote *remote = current2->data;

            if (source != remote->source)
                continue;

            removeremote(remote);
            list_move(&remotelist, &view->remotes, &remote->item);

            view->currentremote = (view->remotes.tail) ? view->remotes.tail->data : 0;

            if (view->currentremote)
                activateremote(view->currentremote);

            arrangeview(channel, view);

            if (view == currentview)
                showremotes(channel, &view->remotes);

        }

    }

}

static void onwmrenderdata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct list_item *current;

    for (current = currentview->remotes.head; current; current = current->next)
    {

        struct remote *remote = current->data;

        if (remote->source == source)
        {

            unsigned int x = remote->window.size.x + 2 + padding;
            unsigned int y = remote->window.size.y + 2 + padding;
            unsigned int w = remote->window.size.w - 4 - padding * 2;
            unsigned int h = remote->window.size.h - 4 - padding * 2;

            render_write(source, mdata, msize);
            render_resize(source, x, y, w, h, padding, lineheight, steplength);
            render_flush(canvasdata, 0x10000, draw);
            render_complete();

        }

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G1);
    file_link(FILE_G2);
    file_link(FILE_G3);
    file_link(FILE_G5);
    file_open(FILE_G6);
    setupvideo();
    setupviews();
    setupremotes();
    activateview(currentview);

    while (channel_poll(channel, &header, &data))
    {

        channel_dispatch(channel, &header, &data);

        if (ring_count(&output))
        {

            char buffer[BUFFER_SIZE];
            unsigned int count = ring_read(&output, buffer, BUFFER_SIZE);

            render_write(0, buffer, count);
            render_resize(0, screen.x, screen.y, screen.w, screen.h, padding, lineheight, steplength);
            render_flush(canvasdata, 0x10000, draw);
            render_complete();

        }

    }

    file_close(FILE_G6);
    file_unlink(FILE_G5);
    file_unlink(FILE_G3);
    file_unlink(FILE_G2);
    file_unlink(FILE_G1);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G4, mdata))
        return;

    if (!file_walk(FILE_G5, FILE_G4, "event"))
        return;

    if (!file_walk(FILE_G6, FILE_G4, "data"))
        return;

}

void init(struct channel *channel)
{

    list_init(&viewlist);
    list_init(&remotelist);
    ring_init(&output, BUFFER_SIZE, outputdata);
    widget_initfill(&background, 2);
    widget_initmouse(&mouse, WIDGET_MOUSETYPE_DEFAULT);
    render_init();

    if (!file_walk2(FILE_G1, "system:wm"))
        return;

    if (!file_walk2(FILE_G2, "system:keyboard/event"))
        return;

    if (!file_walk2(FILE_G3, "system:mouse/event"))
        return;

    if (!file_walk2(FILE_G4, "system:video/if:0"))
        return;

    if (!file_walk(FILE_G5, FILE_G4, "event"))
        return;

    if (!file_walk(FILE_G6, FILE_G4, "data"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_FILE, onfile);
    channel_setcallback(channel, EVENT_KEYPRESS, onkeypress);
    channel_setcallback(channel, EVENT_KEYRELEASE, onkeyrelease);
    channel_setcallback(channel, EVENT_MOUSEMOVE, onmousemove);
    channel_setcallback(channel, EVENT_MOUSESCROLL, onmousescroll);
    channel_setcallback(channel, EVENT_MOUSEPRESS, onmousepress);
    channel_setcallback(channel, EVENT_MOUSERELEASE, onmouserelease);
    channel_setcallback(channel, EVENT_VIDEOMODE, onvideomode);
    channel_setcallback(channel, EVENT_WMMAP, onwmmap);
    channel_setcallback(channel, EVENT_WMUNMAP, onwmunmap);
    channel_setcallback(channel, EVENT_WMRENDERDATA, onwmrenderdata);

}

