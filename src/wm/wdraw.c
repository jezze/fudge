#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "ev.h"
#include "render.h"

static struct ctrl_videosettings settings;
static struct ev_handlers handlers;

static void onwmmap(struct event_header *header)
{

    unsigned int factor;
    unsigned int lineheight;

    ctrl_setvideosettings(&settings, 1920, 1080, 32);
    render_initvideo(CALL_L6, &settings);
    render_initcolormap(CALL_L7);

    factor = (settings.w + (160 - 1)) / 160;

    switch (factor)
    {

    case 12:
    case 11:
    case 10:
        lineheight = 18 + 6;

        render_initmouse(24);
        file_walk(CALL_L8, "/share/ter-118n.pcf");
        render_initfont(CALL_L8, lineheight);

        break;

    case 9:
    case 8:
    case 7:
        lineheight = 16 + 4;

        render_initmouse(24);
        file_walk(CALL_L8, "/share/ter-116n.pcf");
        render_initfont(CALL_L8, lineheight);

        break;

    case 6:
    case 5:
    case 4:
        lineheight = 14 + 2;

        render_initmouse(16);
        file_walk(CALL_L8, "/share/ter-114n.pcf");
        render_initfont(CALL_L8, lineheight);

        break;

    case 3:
    case 2:
    case 1:
    default:
        lineheight = 12 + 0;

        render_initmouse(16);
        file_walk(CALL_L8, "/share/ter-112n.pcf");
        render_initfont(CALL_L8, lineheight);

        break;

    }

    ev_sendwmresize(CALL_L1, header->source, 0, 0, settings.w, settings.h, factor + 2, lineheight);
    ev_sendwmshow(CALL_L1, header->source);

    handlers.wmmap = 0;

}

static void onwmflush(struct event_header *header)
{

    render_parse(CALL_L0);
    render_update(CALL_L5, &settings);
    render_complete();

}

void main(void)
{

    handlers.wmmap = onwmmap;
    handlers.wmflush = onwmflush;

    render_init();

    if (!file_walk(CALL_L0, "/system/wm/data"))
        return;

    if (!file_walk(CALL_L1, "/system/wm/event"))
        return;

    if (!file_walk(CALL_L5, "/system/video/if:0/data"))
        return;

    if (!file_walk(CALL_L6, "/system/video/if:0/ctrl"))
        return;

    if (!file_walk(CALL_L7, "/system/video/if:0/colormap"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);

    while (ev_read(&handlers, CALL_L1));

    file_close(CALL_L1);
    file_close(CALL_L0);

}

