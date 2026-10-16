#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define NUM_MODES                       2

static char path[256];
static unsigned int wm;
static char *labels[NUM_MODES] = {"Normal", "Stretched"};
static char *flows[NUM_MODES] = {"vertical", "stretch"};
static unsigned int mode;

static void showimage(void)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');

    if (!wm || !length)
        return;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"%s\"\n= frame flow \"%s\"\n+ image id \"image\" in \"frame\" mimetype \"image/pcx\" source \"%s\"\n", path + start, flows[mode], path);

}

static void choose(void)
{

    unsigned int target = fs_spawn(1, 0, "initrd:bin/wfile");

    if (target)
    {

        channel_send_fmt(0, target, EVENT_OPTION, "mode\\0select\\0wm-service\\0%s\\0", option_getstring("wm-service"));
        channel_send(0, target, EVENT_MAIN, 0, 0);
        channel_send(0, target, EVENT_TERM, 0, 0);

    }

}

static void onmain(struct message *message)
{

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wimage.alfi";

        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        showimage();
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

    }

}

static void onpath(struct message *message)
{

    if (path[0])
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "- image\n");

    cstring_write_fmt(path, 256, 0, "%s\\0", message->data);
    showimage();

}

static void onwmevent(struct message *message)
{

    struct event_wmevent *event = message->data;

    if (kv_match(event, "q=mode"))
    {

        char *label = kv_getstring(event, "mode=");
        unsigned int i;

        for (i = 0; i < NUM_MODES; i++)
        {

            if (label && cstring_match(label, labels[i]))
                mode = i;

        }

        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= mode label \"%s\"\n= frame flow \"%s\"\n", labels[mode], flows[mode]);

    }

    if (kv_match(event, "q=open"))
        choose();

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_WMEVENT, onwmevent);

}

