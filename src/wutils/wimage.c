#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define NUM_MODES                       3

static char path[256];
static unsigned int wm;
static char *labels[NUM_MODES] = {"Normal", "Stretched", "Filled"};
static char *modes[NUM_MODES] = {"normal", "stretch", "fill"};
static unsigned int mode;

static void showimage(void)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');

    if (!wm || !length)
        return;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"%s\"\n+ image id \"image\" in \"frame\" mimetype \"image/pcx\" source \"%s\" mode \"%s\" span \"1\"\n", path + start, path, modes[mode]);

}

static void onmain(struct message *message)
{

    unsigned int target = channel_lookup(option_getstring("wm-service"));

    if (target)
    {

        channel_send(0, target, EVENT_WMMAP, 0, 0);
        channel_hold(0);
        channel_send(0, target, EVENT_WMUNMAP, 0, 0);

    }

}

static void onpath(struct message *message)
{

    if (path[0])
        return;

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

        channel_send_fmt(0, message->source, EVENT_WMRENDERDATA, "= mode label \"%s\"\n= image mode \"%s\"\n", labels[mode], modes[mode]);

    }

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wimage.alfi";

    wm = message->source;

    channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    showimage();

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_WMEVENT, onwmevent);
    channel_bind(EVENT_WMINIT, onwminit);

}

