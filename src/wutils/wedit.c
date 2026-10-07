#include <fudge.h>
#include <abi.h>

#define CHUNKSIZE                       768
#define MAXSIZE                         0x4000

static char path[256];
static unsigned int wm;

/* a piece of the file as an inline text, so the pieces run on as one text; a quote is written as "\"" */
static void sendchunk(char *data, unsigned int count)
{

    char buffer[MESSAGE_SIZE];
    unsigned int offset = cstring_write_fmt0(buffer, MESSAGE_SIZE, 0, "+ text in \"content\" display \"inline\" wrap \"char\" label \"");
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (data[i] == '"')
            offset += buffer_write(buffer, MESSAGE_SIZE, "\"\\\"\"", 4, offset);
        else
            offset += buffer_write(buffer, MESSAGE_SIZE, &data[i], 1, offset);

    }

    offset += buffer_write(buffer, MESSAGE_SIZE, "\"\n", 2, offset);

    channel_send(0, wm, EVENT_WMRENDERDATA, offset, buffer);

}

/* the file is shown once both the path and the window are there, whichever comes last */
static void showfile(void)
{

    unsigned int length = cstring_length(path);
    unsigned int start = buffer_lastbyte(path, length, '/');
    unsigned int target;
    unsigned int id;

    if (!wm || !length)
        return;

    if (!start)
        start = buffer_firstbyte(path, length, ':');

    channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "= window label \"%s\"\n", path + start);

    target = fs_auth(path);
    id = (target) ? fs_walk(1, target, 0, path) : 0;

    if (id)
    {

        char data[CHUNKSIZE];
        unsigned int offset = 0;
        unsigned int count;

        while (offset < MAXSIZE && (count = fs_read(1, target, id, data, CHUNKSIZE, offset)))
        {

            sendchunk(data, count);

            offset += count;

        }

        if (offset >= MAXSIZE)
            channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ text in \"content\" weight \"bold\" label \"(only the first 16 KB are shown)\"\n");

        /* the textbox draws its cursor in its last text: an empty one puts it at the end of the file */
        channel_send_fmt0(0, wm, EVENT_WMRENDERDATA, "+ text in \"content\" display \"inline\" wrap \"char\"\n");

    }

    else
    {

        channel_send_fmt1(0, wm, EVENT_WMRENDERDATA, "+ text in \"content\" label \"Could not open %s\"\n", path);

    }

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

    cstring_write_fmt1(path, 256, 0, "%s\\0", message->data);
    showfile();

}

static void onwminit(struct message *message)
{

    char *alfi = "initrd:data/alfi/wedit.alfi";

    wm = message->source;

    channel_send(0, wm, EVENT_WMRENDERFILE, cstring_length_zero(alfi), alfi);
    showfile();

}

void init(void)
{

    option_add("wm-service", "wm");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_PATH, onpath);
    channel_bind(EVENT_WMINIT, onwminit);

}
