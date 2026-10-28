#include <fudge.h>
#include <abi.h>
#include "emulator.h"
#include "glue.h"

#define TICKRATE                        60
#define CATCHUP                         3
#define AUDIOCUSHION                    3
#define AUDIOLIMIT                      8
#define HELD_SPACE                      1
#define HELD_F5                         2

static Emulator emulator;
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
static short audiobuffer[AUDIO_BUFF_SIZE * 2];
static unsigned int scaled[4096];

static unsigned int getbutton(unsigned int id)
{

    switch (id)
    {

    case KEYS_KEY_CURSORRIGHT:
        return RIGHT;

    case KEYS_KEY_CURSORLEFT:
        return LEFT;

    case KEYS_KEY_CURSORDOWN:
        return DOWN;

    case KEYS_KEY_CURSORUP:
        return UP;

    case KEYS_KEY_ENTER:
        return START;

    case KEYS_KEY_RSHIFT:
        return SELECT;

    case KEYS_KEY_J:
        return BUTTON_A;

    case KEYS_KEY_K:
        return BUTTON_B;

    case KEYS_KEY_H:
        return TURBO_A;

    case KEYS_KEY_L:
        return TURBO_B;

    }

    return 0;

}

/* both controllers take the keyboard, as upstream does; start and select together reset */
static void setbutton(unsigned int id, unsigned int pressed)
{

    unsigned int button = getbutton(id);

    if (!button)
        return;

    update_joypad(&emulator.mem.joy1, button, pressed);
    update_joypad(&emulator.mem.joy2, button, pressed);

    if ((emulator.mem.joy1.status & 0xC) == 0xC)
        reset_emulator(&emulator);

}

/* the 256x240 screen at the largest whole scale that fits, centered */
static void present(void)
{

    unsigned int scalew = fbwidth / VISIBLE_DOTS;
    unsigned int scaleh = fbheight / VISIBLE_SCANLINES;
    unsigned int scale = (scalew < scaleh) ? scalew : scaleh;
    unsigned int totw = VISIBLE_DOTS * scale;
    unsigned int toth = VISIBLE_SCANLINES * scale;
    unsigned int offx = (fbwidth - totw) / 2;
    unsigned int offy = (fbheight - toth) / 2;
    struct event_videoflush videoflush;
    unsigned int y;

    if (!framebuffer || !scale || totw > 4096)
        return;

    for (y = 0; y < VISIBLE_SCANLINES; y++)
    {

        unsigned int *row = emulator.ppu.screen + y * VISIBLE_DOTS;
        unsigned int x;
        unsigned int i;

        for (x = 0; x < VISIBLE_DOTS; x++)
        {

            for (i = 0; i < scale; i++)
                scaled[x * scale + i] = row[x];

        }

        for (i = 0; i < scale; i++)
            buffer_copy(framebuffer + (offy + y * scale + i) * fbwidth + offx, scaled, totw * 4);

    }

    videoflush.x = offx;
    videoflush.y = offy;
    videoflush.width = totw;
    videoflush.height = toth;

    channel_send(0, video, EVENT_VIDEOFLUSH, sizeof (struct event_videoflush), &videoflush);

}

/* mono samples go out as 16-bit stereo, unless the device already holds more than AUDIOLIMIT ticks */
static void sendaudio(short *samples, unsigned int count)
{

    unsigned int i;

    if (audioahead > (int)(rate / TICKRATE * AUDIOLIMIT))
        return;

    for (i = 0; i < count; i++)
    {

        audiobuffer[i * 2] = (samples) ? samples[i] : 0;
        audiobuffer[i * 2 + 1] = audiobuffer[i * 2];

    }

    channel_send(1, audio, EVENT_DATA, count * 4, audiobuffer);

    audioahead += count;

}

/* the device plays rate / TICKRATE samples a tick; silence tops it up to AUDIOCUSHION ticks after a pause or a stall */
static void fillaudio(void)
{

    while (audioahead < (int)(rate / TICKRATE * AUDIOCUSHION))
        sendaudio(0, rate / TICKRATE);

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

    unsigned int rateframes = (emulator.type == PAL) ? PAL_FRAME_RATE : NTSC_FRAME_RATE;

    while (channel_drain(0))
    {

        unsigned int due = ticks * rateframes / TICKRATE;

        if (paused || due - frames > CATCHUP)
            frames = (paused) ? due : due - 1;

        if (audio)
            fillaudio();

        if (frames < due)
        {

            run_frame(&emulator);

            if (audio)
                sendaudio(emulator.mixer.buff, emulator.mixer.index);

            emulator.mixer.index = 0;
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

    case KEYS_KEY_SPACE:
        if (!(held & HELD_SPACE))
            paused = !paused;

        held |= HELD_SPACE;

        break;

    case KEYS_KEY_F5:
        if (!(held & HELD_F5))
            reset_emulator(&emulator);

        held |= HELD_F5;

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

    case KEYS_KEY_SPACE:
        held &= ~HELD_SPACE;

        break;

    case KEYS_KEY_F5:
        held &= ~HELD_F5;

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

/* 48000 Hz like upstream, else 44100 Hz; both are whole samples per tick */
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

    }

}

/* the whole file stays on the heap, the mapper points into it */
static unsigned int load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;
    ROMData data;

    if (!id || !fs_stat(1, target, id, &record))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Path not found: %s\n", path);

        return 0;

    }

    data.rom_name = path;
    data.rom_size = record.size;
    data.rom = malloc(record.size);

    if (!data.rom || fs_read_full(1, target, id, data.rom, record.size, 0) != record.size)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Could not read: %s\n", path);

        return 0;

    }

    if (init_emulator(&emulator, &data, (audio) ? rate : 48000) < 0)
    {

        unsigned int mapper = emulator.mapper.mapper_num;

        channel_send_fmt(0, source, EVENT_ERROR, "Not a supported NES ROM (mapper %u): %s\n", &mapper, path);

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

