#include <fudge.h>
#include <abi.h>

static char path[256];
static unsigned int wm;

/* the image is added once both the path and the window are there, whichever comes last */
static void showimage(void)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');

    if (!wm || !length)
        return;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    channel_send_fmt(0, wm, EVENT_WMRENDERDATA, "= window label \"%s\"\n+ image in \"base\" mimetype \"image/pcx\" source \"%s\"\n", path + start, path);

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
    channel_bind(EVENT_WMINIT, onwminit);

}
