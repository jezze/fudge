#include <fudge.h>
#include <abi.h>
#include "kv.h"

#define NUM_MODES                       2
#define NUM_NAMES                       16

static char path[256];
static unsigned int wm;
static char *labels[NUM_MODES] = {"Normal", "Stretched"};
static char *flows[NUM_MODES] = {"vertical", "stretch"};
static unsigned int mode;
static unsigned int memtarget;
static unsigned int memparent;
static unsigned int memid;
static char memname[32];

static void createmem(void)
{

    unsigned int i;

    memtarget = fs_auth("mem:");
    memparent = (memtarget) ? fs_walk(1, memtarget, 0, "mem:") : 0;

    for (i = 0; i < NUM_NAMES && memparent && !memid; i++)
    {

        cstring_write_fmt(memname, 32, 0, "wimage%u\\0", &i);

        memid = fs_create(1, memtarget, memparent, memname, cstring_length(memname));

    }

}

static void showimage(void)
{

    char size[64];
    unsigned int count;

    if (!wm || !path[0])
        return;

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"Image Viewer (%s)\"\n= frame flow \"%s\"\n", path + fs_dirlength(path), flows[mode]);

    if (!memid)
        return;

    count = system_feed(0, 0, size, 63, "mapimage %s mem:%s", path, memname);
    size[count] = 0;

    if (count)
        channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "+ image id \"image\" in \"frame\" mimetype \"image/raw\" source \"mem:%s\" %s", memname, size);

}

static void choose(void)
{

    unsigned int target = fs_spawn(1, "initrd:bin/wfile");

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

        createmem();
        channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
        showimage();
        channel_hold(0);
        channel_send(0, wm, EVENT_WMUNMAP, 0, 0);

        if (memid)
            fs_remove(1, memtarget, memparent, memid);

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

