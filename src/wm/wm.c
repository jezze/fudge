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
static unsigned int rendertarget;

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

static void showremotes(union event_message *imessage, union event_message *omessage, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(omessage, imessage, EVENT_WMSHOW, remote->source, 0);
        event_send(omessage);
        updateremote(imessage, remote);

    }

}

static void hideremotes(union event_message *imessage, union event_message *omessage, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(omessage, imessage, EVENT_WMHIDE, remote->source, 0);
        event_send(omessage);
        removeremote(imessage, remote);

    }

}

static void configureremotes(union event_message *imessage, union event_message *omessage, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(omessage, imessage, EVENT_WMCONFIGURE, remote->source, 0);
        event_addwmconfigure(omessage, imessage->header.target, remote->window.size.x + 2, remote->window.size.y + 2, remote->window.size.w - 4, remote->window.size.h - 4, padding, lineheight);
        event_send(omessage);

    }

}

static void killremotes(union event_message *imessage, union event_message *omessage, struct list *remotes)
{

    struct list_item *current;

    for (current = remotes->head; current; current = current->next)
    {

        struct remote *remote = current->data;

        event_request(omessage, imessage, EVENT_KILL, remote->source, 0);
        event_send(omessage);

    }

}

static void flipview(union event_message *imessage, union event_message *omessage, struct view *view)
{

    deactivateview(currentview);
    hideremotes(imessage, omessage, &currentview->remotes);
    updateview(imessage, currentview);

    currentview = view;

    activateview(currentview);
    showremotes(imessage, omessage, &currentview->remotes);
    updateview(imessage, currentview);

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

static void arrangeview(union event_message *imessage, union event_message *omessage, struct view *view)
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

    configureremotes(imessage, omessage, &view->remotes);

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

static unsigned int ondata(union event_message *imessage, union event_message *omessage)
{

    struct event_data *data = event_getdata(imessage);

    if (!file_walk(FILE_L0, FILE_G2, "../data"))
        return 0;

    render_write(data + 1, data->count);
    render_flush(FILE_L0);
    render_complete();

    return 0;

}

static unsigned int oninit(union event_message *imessage, union event_message *omessage)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);
    widget_initfill(&background, 2);
    widget_initmouse(&mouse, WIDGET_MOUSETYPE_DEFAULT);
    setupviews();
    setupremotes();
    activateview(currentview);
    render_init();

    if (!file_walk(FILE_L0, FILE_G2, "../ctrl"))
        return 1;

    render_setvideo(FILE_L0, 1024, 768, 4);

    if (!file_walk(FILE_L0, FILE_G2, "../colormap"))
        return 1;

    render_setcolormap(FILE_L0);

    return 0;

}

static unsigned int onkill(union event_message *imessage, union event_message *omessage)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        hideremotes(imessage, omessage, &view->remotes);
        killremotes(imessage, omessage, &view->remotes);

    }

    return 1;

}

static unsigned int onkeypress(union event_message *imessage, union event_message *omessage)
{

    struct event_keypress *keypress = event_getdata(imessage);
    struct view *nextview;
    struct remote *nextremote;
    unsigned int id;

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            event_request(omessage, imessage, EVENT_WMKEYPRESS, currentview->currentremote->source, 0);
            event_addwmkeypress(omessage, keypress->scancode);
            event_send(omessage);

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

            arrangeview(imessage, omessage, currentview);
            arrangeview(imessage, omessage, nextview);

            if (nextview->currentremote)
                deactivateremote(nextview->currentremote);

            nextview->currentremote = (nextview->remotes.tail) ? nextview->remotes.tail->data : 0;

        }

        flipview(imessage, omessage, nextview);

        break;

    case 0x10:
        if (!currentview->currentremote)
            break;

        if ((keymod & KEYMOD_SHIFT))
        {

            event_request(omessage, imessage, EVENT_WMHIDE, currentview->currentremote->source, 0);
            event_send(omessage);
            event_request(omessage, imessage, EVENT_KILL, currentview->currentremote->source, 0);
            event_send(omessage);

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

            event_request(omessage, imessage, EVENT_INIT, id, 0);
            event_send(omessage);
            event_request(omessage, imessage, EVENT_STOP, id, 0);
            event_send(omessage);

        }

        break;

    case 0x1C:
        if (!currentview->currentremote)
            break;

        list_move(&currentview->remotes, &currentview->remotes, &currentview->currentremote->item);
        arrangeview(imessage, omessage, currentview);
        showremotes(imessage, omessage, &currentview->remotes);

        break;

    case 0x23:
        if (currentview->center <= 6 * steplength)
            break;

        currentview->center -= 4 * steplength;

        arrangeview(imessage, omessage, currentview);
        showremotes(imessage, omessage, &currentview->remotes);

        break;

    case 0x24:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.next ? currentview->currentremote->item.next->data : currentview->remotes.head;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(imessage, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(imessage, currentview->currentremote);

        break;

    case 0x25:
        if (!currentview->currentremote)
            break;

        nextremote = currentview->currentremote->item.prev ? currentview->currentremote->item.prev->data : currentview->remotes.tail;

        if (!nextremote || nextremote == currentview->currentremote)
            break;

        deactivateremote(currentview->currentremote);
        updateremote(imessage, currentview->currentremote);

        currentview->currentremote = nextremote;

        activateremote(currentview->currentremote);
        updateremote(imessage, currentview->currentremote);

        break;

    case 0x26:
        if (currentview->center >= 26 * steplength)
            break;

        currentview->center += 4 * steplength;

        arrangeview(imessage, omessage, currentview);
        showremotes(imessage, omessage, &currentview->remotes);

        break;

    case 0x2C:
        if ((keymod & KEYMOD_SHIFT))
        {

            /* Sending to self not allowed */
            event_request(omessage, imessage, EVENT_WMHIDE, imessage->header.target, 0);
            event_send(omessage);
            /* Sending to self not allowed */
            event_request(omessage, imessage, EVENT_KILL, imessage->header.target, 0);
            event_send(omessage);

        }

        break;

    }

    return 0;

}

static unsigned int onkeyrelease(union event_message *imessage, union event_message *omessage)
{

    struct event_keyrelease *keyrelease = event_getdata(imessage);

    keymod = keymap_modkey(keyrelease->scancode, keymod);

    if (!(keymod & KEYMOD_ALT))
    {

        if (currentview->currentremote)
        {

            event_request(omessage, imessage, EVENT_WMKEYRELEASE, currentview->currentremote->source, 0);
            event_addwmkeyrelease(omessage, keyrelease->scancode);
            event_send(omessage);

        }

        return 0;

    }

    return 0;

}

static unsigned int onmousemove(union event_message *imessage, union event_message *omessage)
{

    struct event_mousemove *mousemove = event_getdata(imessage);

    mouse.size.x += mousemove->relx;
    mouse.size.y += mousemove->rely;

    if (mouse.size.x < screen.x || mouse.size.x >= screen.x + screen.w)
        mouse.size.x = (mousemove->relx < 0) ? screen.x : screen.x + screen.w - 1;

    if (mouse.size.y < screen.y || mouse.size.y >= screen.y + screen.h)
        mouse.size.y = (mousemove->rely < 0) ? screen.y : screen.y + screen.h - 1;

    updatemouse(imessage);

    if (currentview->currentremote)
    {

        event_request(omessage, imessage, EVENT_WMMOUSEMOVE, currentview->currentremote->source, 0);
        event_addwmmousemove(omessage, mouse.size.x, mouse.size.y);
        event_send(omessage);

    }

    return 0;

}

static unsigned int onmousepress(union event_message *imessage, union event_message *omessage)
{

    struct event_mousepress *mousepress = event_getdata(imessage);
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

                flipview(imessage, omessage, view);

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
                updateremote(imessage, currentview->currentremote);

                currentview->currentremote = remote;

                activateremote(currentview->currentremote);
                updateremote(imessage, currentview->currentremote);

                break;

            }

        }

        break;

    }

    if (currentview->currentremote)
    {

        event_request(omessage, imessage, EVENT_WMMOUSEPRESS, currentview->currentremote->source, 0);
        event_addwmmousepress(omessage, mousepress->button);
        event_send(omessage);

    }

    return 0;

}

static unsigned int onmouserelease(union event_message *imessage, union event_message *omessage)
{

    struct event_mouserelease *mouserelease = event_getdata(imessage);

    if (currentview->currentremote)
    {

        event_request(omessage, imessage, EVENT_WMMOUSERELEASE, currentview->currentremote->source, 0);
        event_addwmmouserelease(omessage, mouserelease->button);
        event_send(omessage);

    }

    return 0;

}

static unsigned int onvideomode(union event_message *imessage, union event_message *omessage)
{

    struct event_videomode *videomode = event_getdata(imessage);
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

    /* Sending to self not allowed */
    event_request(omessage, imessage, EVENT_WMCONFIGURE, imessage->header.target, 0);
    event_addwmconfigure(omessage, imessage->header.target, 0, 0, videomode->w, videomode->h, padding, lineheight);
    event_send(omessage);
    /* Sending to self not allowed */
    event_request(omessage, imessage, EVENT_WMSHOW, imessage->header.target, 0);
    event_send(omessage);

    return 0;

}

static unsigned int onwmconfigure(union event_message *imessage, union event_message *omessage)
{

    struct event_wmconfigure *wmconfigure = event_getdata(imessage);
    struct list_item *current;
    unsigned int i = 0;

    rendertarget = wmconfigure->rendertarget;

    box_setsize(&screen, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);
    box_setsize(&body, wmconfigure->x, wmconfigure->y + (wmconfigure->lineheight + wmconfigure->padding * 2), wmconfigure->w, wmconfigure->h - (wmconfigure->lineheight + wmconfigure->padding * 2));
    box_setsize(&background.size, wmconfigure->x, wmconfigure->y, wmconfigure->w, wmconfigure->h);

    steplength = wmconfigure->w / 32;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;

        view->center = 18 * steplength;

        box_setsize(&view->panel.size, wmconfigure->x + i * wmconfigure->w / viewlist.count, wmconfigure->y, wmconfigure->w / viewlist.count, (wmconfigure->lineheight + wmconfigure->padding * 2));
        arrangeview(imessage, omessage, view);

        i++;

    }

    mouse.size.x = wmconfigure->x + wmconfigure->w / 4;
    mouse.size.y = wmconfigure->y + wmconfigure->h / 4;

    return 0;

}

static unsigned int onwmmap(union event_message *imessage, union event_message *omessage)
{

    if (currentview->currentremote)
        deactivateremote(currentview->currentremote);

    currentview->currentremote = remotelist.head->data;
    currentview->currentremote->source = imessage->header.source;

    list_move(&currentview->remotes, currentview->currentremote->item.list, &currentview->currentremote->item);
    activateremote(currentview->currentremote);
    arrangeview(imessage, omessage, currentview);
    showremotes(imessage, omessage, &currentview->remotes);

    return 0;

}

static unsigned int onwmunmap(union event_message *imessage, union event_message *omessage)
{

    struct list_item *current;

    for (current = viewlist.head; current; current = current->next)
    {

        struct view *view = current->data;
        struct list_item *current2;

        for (current2 = view->remotes.head; current2; current2 = current2->next)
        {

            struct remote *remote = current2->data;

            if (imessage->header.source != remote->source)
                continue;

            removeremote(imessage, remote);
            list_move(&remotelist, remote->item.list, &remote->item);

            view->currentremote = (view->remotes.tail) ? view->remotes.tail->data : 0;

            if (view->currentremote)
                activateremote(view->currentremote);

            arrangeview(imessage, omessage, view);

            if (view == currentview)
                showremotes(imessage, omessage, &view->remotes);

        }

    }

    return 0;

}

static unsigned int onwmshow(union event_message *imessage, union event_message *omessage)
{

    struct list_item *current;

    updatebackground(imessage);
    updatemouse(imessage);

    for (current = viewlist.head; current; current = current->next)
        updateview(imessage, current->data);

    showremotes(imessage, omessage, &currentview->remotes);

    return 0;

}

static unsigned int onwmhide(union event_message *imessage, union event_message *omessage)
{

    struct list_item *current;

    removebackground(imessage);
    removemouse(imessage);

    for (current = viewlist.head; current; current = current->next)
        removeview(imessage, current->data);

    hideremotes(imessage, omessage, &currentview->remotes);

    return 0;

}

void main(void)
{

    unsigned int status = 0;
    union event_message imessage;
    union event_message omessage;

    if (!file_walk2(FILE_G0, "/system/keyboard/event"))
        return;

    if (!file_walk2(FILE_G1, "/system/mouse/event"))
        return;

    if (!file_walk2(FILE_G2, "/system/video/if:0/event"))
        return;

    file_open(FILE_G0);
    file_open(FILE_G1);
    file_open(FILE_G2);
    event_open();

    while (!status)
    {

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_DATA:
            status = ondata(&imessage, &omessage);

            break;

        case EVENT_INIT:
            status = oninit(&imessage, &omessage);

            break;

        case EVENT_KILL:
            status = onkill(&imessage, &omessage);

            break;

        case EVENT_KEYPRESS:
            status = onkeypress(&imessage, &omessage);

            break;

        case EVENT_KEYRELEASE:
            status = onkeyrelease(&imessage, &omessage);

            break;

        case EVENT_MOUSEMOVE:
            status = onmousemove(&imessage, &omessage);

            break;

        case EVENT_MOUSEPRESS:
            status = onmousepress(&imessage, &omessage);

            break;

        case EVENT_MOUSERELEASE:
            status = onmouserelease(&imessage, &omessage);

            break;

        case EVENT_VIDEOMODE:
            status = onvideomode(&imessage, &omessage);

            break;

        case EVENT_WMCONFIGURE:
            status = onwmconfigure(&imessage, &omessage);

            break;

        case EVENT_WMMAP:
            status = onwmmap(&imessage, &omessage);

            break;

        case EVENT_WMUNMAP:
            status = onwmunmap(&imessage, &omessage);

            break;

        case EVENT_WMSHOW:
            status = onwmshow(&imessage, &omessage);

            break;

        case EVENT_WMHIDE:
            status = onwmhide(&imessage, &omessage);

            break;

        }

        if (ring_count(&output))
        {

            event_request(&omessage, &imessage, EVENT_DATA, rendertarget, 0);
            event_adddata(&omessage);
            event_appenddata(&omessage, ring_count(&output), outputdata);
            event_send(&omessage);
            ring_reset(&output);

        }

    }

    event_close();
    file_close(FILE_G2);
    file_close(FILE_G1);
    file_close(FILE_G0);

}

