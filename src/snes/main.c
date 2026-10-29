#include <fudge.h>
#include <abi.h>
#include "snes.h"
#include "glue.h"

#define TICKRATE                        60
#define CATCHUP                         3
#define AUDIOCUSHION                    3
#define AUDIOLIMIT                      8
#define WIDTH                           256
#define HEIGHT                          240
#define SCALEDWIDTH                     4096
#define AUDIOSIZE                       (48000 / 50 * 2)
#define HELD_PAUSE                      1
#define HELD_RESET                      2

static Snes *snes;
static struct keys keys;
static unsigned int *framebuffer;
static unsigned int fbwidth;
static unsigned int fbheight;
static unsigned int video;
static unsigned int audio;
static unsigned int rate;
static int audioahead;
static unsigned int counter;
static unsigned int ticks;
static unsigned int frames;
static unsigned int counting;
static unsigned int paused;
static unsigned int held;
static unsigned int started;
static short audiobuffer[AUDIOSIZE];
static unsigned int columns[SCALEDWIDTH];
static unsigned int scaled[SCALEDWIDTH];

/* buttons as upstream: B Y Select Start Up Down Left Right A X L R */
static int getbutton(unsigned int id)
{

    switch (id)
    {

    case KEYS_KEY_Z:
        return 0;

    case KEYS_KEY_A:
        return 1;

    case KEYS_KEY_RSHIFT:
        return 2;

    case KEYS_KEY_ENTER:
        return 3;

    case KEYS_KEY_CURSORUP:
        return 4;

    case KEYS_KEY_CURSORDOWN:
        return 5;

    case KEYS_KEY_CURSORLEFT:
        return 6;

    case KEYS_KEY_CURSORRIGHT:
        return 7;

    case KEYS_KEY_X:
        return 8;

    case KEYS_KEY_S:
        return 9;

    case KEYS_KEY_D:
        return 10;

    case KEYS_KEY_C:
        return 11;

    }

    return -1;

}

static void setbutton(unsigned int id, unsigned int pressed)
{

    int button = getbutton(id);

    if (button >= 0)
        snes_setButtonState(snes, 1, button, pressed);

}

static unsigned int getscale(void)
{

    unsigned int scalew = fbwidth / WIDTH;
    unsigned int scaleh = fbheight / HEIGHT;

    return (scalew < scaleh) ? scalew : scaleh;

}

/* screen columns pick from the ppu's 512 half pixels */
static void setupcolumns(void)
{

    unsigned int scale = getscale();
    unsigned int x;

    for (x = 0; x < WIDTH * scale && x < SCALEDWIDTH; x++)
        columns[x] = x * 2 / scale;

}

static void scalerow(unsigned int width, unsigned int y)
{

    unsigned int *row = (unsigned int *)snes->ppu->pixelBuffer + y * 512;
    unsigned int x;

    for (x = 0; x < width; x++)
        scaled[x] = row[columns[x]];

}

/* the 240 lines as upstream shows them (224 lines 8 down unless overscanning, interlaced frames take both fields) at the largest whole scale that fits, centered */
static void present(void)
{

    unsigned int scale = getscale();
    unsigned int totw = WIDTH * scale;
    unsigned int toth = HEIGHT * scale;
    unsigned int offx = (fbwidth - totw) / 2;
    unsigned int offy = (fbheight - toth) / 2;
    unsigned int lines = (snes->ppu->frameOverscan) ? 239 : 224;
    unsigned int top = (snes->ppu->frameOverscan) ? 1 : 8;
    struct event_videoflush videoflush;
    unsigned int line;

    if (!framebuffer || !scale || totw > SCALEDWIDTH)
        return;

    for (line = 0; line < HEIGHT; line++)
    {

        unsigned int field = 2;
        unsigned int i;

        for (i = 0; i < scale; i++)
        {

            unsigned int *out = framebuffer + (offy + line * scale + i) * fbwidth + offx;
            unsigned int y = line - top;
            unsigned int current = (i * 2) / scale;

            if (line < top || y >= lines)
            {

                buffer_clear(out, totw * 4);

                continue;

            }

            if (current != field)
            {

                if (snes->ppu->frameInterlace)
                    scalerow(totw, y + current * 239);
                else
                    scalerow(totw, y + ((snes->ppu->evenFrame) ? 0 : 239));

                field = current;

            }

            buffer_copy(out, scaled, totw * 4);

        }

    }

    videoflush.x = offx;
    videoflush.y = offy;
    videoflush.width = totw;
    videoflush.height = toth;

    channel_send(0, video, EVENT_VIDEOFLUSH, sizeof (struct event_videoflush), &videoflush);

}

static unsigned int getframerate(void)
{

    return (snes->palTiming) ? 50 : 60;

}

/* samples go out as 16-bit stereo, unless the device already holds more than AUDIOLIMIT ticks */
static void sendaudio(unsigned int count)
{

    if (audioahead > (int)(rate / TICKRATE * AUDIOLIMIT))
        return;

    channel_send(1, audio, EVENT_DATA, count * 4, audiobuffer);

    audioahead += count;

}

/* the device plays rate / TICKRATE samples a tick; silence tops it up to AUDIOCUSHION ticks after a pause or a stall */
static void fillaudio(void)
{

    buffer_clear(audiobuffer, rate / TICKRATE * 4);

    while (audioahead < (int)(rate / TICKRATE * AUDIOCUSHION))
        sendaudio(rate / TICKRATE);

}

/* the clock is the timer's counter, so ticks dropped while a frame ran still count; the device plays as much meanwhile */
static void ontimertick(struct message *message)
{

    struct event_timertick *timertick = message->data;
    unsigned int elapsed;

    if (!counting)
    {

        counter = timertick->counter;
        counting = 1;

    }

    elapsed = timertick->counter - counter;
    counter = timertick->counter;
    ticks += elapsed;

    if (audio)
    {

        audioahead -= elapsed * (rate / TICKRATE);

        if (audioahead < 0)
            audioahead = 0;

    }

}

/* frames follow the timer, and what arrived during one (ticks, keys) is taken before the next, so the mailbox never fills and drops keys; a stall of more than CATCHUP frames is skipped */
static void run(void)
{

    while (channel_drain(0))
    {

        unsigned int due = ticks * getframerate() / TICKRATE;

        if (paused || due - frames > CATCHUP)
            frames = (paused) ? due : due - 1;

        if (audio)
            fillaudio();

        if (frames < due)
        {

            unsigned int count = rate / getframerate();

            snes_runFrame(snes);

            /* also without a device, so the dsp's count of new samples stays one frame's */
            snes_setSamples(snes, audiobuffer, (audio) ? count : 1);

            if (audio)
                sendaudio(count);

            frames++;

            if (frames == due)
                present();

        }

        else if (!channel_process(0))
            break;

    }

}

static void onkeypress(struct message *message)
{

    struct event_keypress *keypress = message->data;
    unsigned int id = keys_getcode(&keys, keypress->scancode);

    switch (id)
    {

    case KEYS_KEY_ESCAPE:
        channel_close(0);

        break;

    case KEYS_KEY_P:
        if (!(held & HELD_PAUSE))
            paused = !paused;

        held |= HELD_PAUSE;

        break;

    case KEYS_KEY_R:
    case KEYS_KEY_E:
        if (!(held & HELD_RESET))
            snes_reset(snes, id == KEYS_KEY_E);

        held |= HELD_RESET;

        break;

    default:
        setbutton(id, 1);

        break;

    }

}

static void onkeyrelease(struct message *message)
{

    struct event_keyrelease *keyrelease = message->data;
    unsigned int id = keys_getcode(&keys, keyrelease->scancode);

    switch (id)
    {

    case KEYS_KEY_P:
        held &= ~HELD_PAUSE;

        break;

    case KEYS_KEY_R:
    case KEYS_KEY_E:
        held &= ~HELD_RESET;

        break;

    default:
        setbutton(id, 0);

        break;

    }

}

static unsigned int setaudioformat(unsigned int value)
{

    struct event_audioconf audioconf;

    audioconf.rate = value;
    audioconf.channels = 2;
    audioconf.bits = 16;

    return channel_send(1, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf);

}

/* 48000 Hz like upstream, else 44100 Hz; both are whole samples per tick and per frame */
static void setupaudio(void)
{

    audio = channel_lookup(option_getstring("audio-service"));

    if (audio && setaudioformat(48000))
        rate = 48000;
    else if (audio && setaudioformat(44100))
        rate = 44100;
    else
        audio = 0;

}

static void setupvideo(void)
{

    struct event_videoconf videoconf;
    struct event_videoinfo videoinfo;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    channel_send(0, video, EVENT_VIDEOCONF, sizeof (struct event_videoconf), &videoconf);
    channel_send(1, video, EVENT_INFO, 0, 0);

    if (channel_wait(1, video, EVENT_VIDEOINFO, sizeof (struct event_videoinfo), &videoinfo) && videoinfo.bpp == 4)
    {

        framebuffer = (unsigned int *)videoinfo.framebuffer;
        fbwidth = videoinfo.width;
        fbheight = videoinfo.height;

        setupcolumns();

    }

}

/* the file is read onto the heap, which the cart's copy (padded to a power of 2) follows */
static unsigned int load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;
    unsigned char *data;

    if (!id || !fs_stat(1, target, id, &record))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return 0;

    }

    data = malloc(record.size);
    snes = snes_init();

    if (!data || !snes || fs_read_full(1, target, id, data, record.size, 0) != record.size)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Could not read: %s\n", path);

        return 0;

    }

    if (!snes_loadRom(snes, data, record.size))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Not a supported SNES ROM: %s\n", path);

        return 0;

    }

    return 1;

}

static void onpath(struct message *message)
{

    unsigned int keyboard = channel_lookup(option_getstring("keyboard-service"));
    unsigned int timer = channel_lookup(option_getstring("timer-service"));
    unsigned int wm = channel_lookup(option_getstring("wm-service"));

    if (started)
        return;

    started = 1;
    video = channel_lookup(option_getstring("video-service"));

    if (!keyboard || !timer || !video)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Could not find the keyboard, timer or video service\n");

        return;

    }

    if (!port_init())
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "No mem: file could be mapped\n");

        return;

    }

    setupaudio();

    if (load(message->source, message->data))
    {

        if (wm)
        {

            channel_send(1, wm, EVENT_WMGRAB, 0, 0);
            channel_wait(1, wm, EVENT_WMACK, 0, 0);

        }

        setupvideo();
        channel_bind(EVENT_TIMERTICK, ontimertick);
        channel_bind(EVENT_KEYPRESS, onkeypress);
        channel_bind(EVENT_KEYRELEASE, onkeyrelease);
        channel_send(0, keyboard, EVENT_LINK, 0, 0);
        channel_send(0, timer, EVENT_LINK, 0, 0);
        run();
        channel_send(0, keyboard, EVENT_UNLINK, 0, 0);
        channel_send(0, timer, EVENT_UNLINK, 0, 0);

        if (wm)
        {

            channel_send(1, wm, EVENT_WMUNGRAB, 0, 0);
            channel_wait(1, wm, EVENT_WMACK, 0, 0);

        }

    }

    port_destroy();

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "4");
    option_add("audio-service", "audio");
    option_add("keyboard-service", "keyboard");
    option_add("timer-service", "timer");
    option_add("video-service", "video");
    option_add("wm-service", "wm");
    channel_bind(EVENT_PATH, onpath);

}

