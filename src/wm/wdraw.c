#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "ev.h"
#include "render.h"

static struct ev_handlers handlers;

static void onwmmap(struct event_header *header)
{

    render_initvideo(CALL_L6);
    render_initcolormap(CALL_L7);
    render_initfont(CALL_L8);

    handlers.wmmap = 0;

}

static void onwmflush(struct event_header *header)
{

    render_flush(CALL_L0, CALL_L5);

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

    if (!file_walk(CALL_L8, "/share/ter-118n.pcf"))
        return;

    file_open(CALL_L0);
    file_open(CALL_L1);

    while (ev_read(&handlers, CALL_L1));

    file_close(CALL_L1);
    file_close(CALL_L0);

}

