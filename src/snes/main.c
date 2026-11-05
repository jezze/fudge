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
#define BUTTONS                         12
#define SCREENS                         16
#define SCREENSIZE                      (WIDTH * HEIGHT * 4)

static Snes *snes;
static struct keys keys;
static unsigned int *framebuffer;
static unsigned int fbwidth;
static unsigned int fbheight;
static unsigned int video;
static unsigned int audio;
static unsigned int keyboard;
static unsigned int timer;
static unsigned int wm;
static unsigned int fullscreen;
static unsigned int wanted;
static unsigned int flushing;
static unsigned int dirty;
static unsigned int screentarget;
static unsigned int screenid;
static unsigned int *screen;
static char screenname[32];
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

static void setupcolumns(unsigned int scale)
{

    unsigned int x;

    for (x = 0; x < WIDTH * scale && x < SCALEDWIDTH; x++)
        columns[x] = x * 2 / scale;

}

static void scalerow(unsigned int width, unsigned int y)
{

    unsigned int *row = (unsigned int *)snes->ppu->pixelBuffer + y * 512;
    unsigned int x;

    for (x = 0; x < width; x++)
        scaled[x] = row[columns[x]] | 0xFF000000;

}

static void flush(void)
{

    char *id = "screen";

    if (flushing)
    {

        dirty = 1;

        return;

    }

    channel_send(0, wm, MESSAGE_TYPE_WMFLUSH, cstring_length_zero(id), id);

    flushing = 1;
    dirty = 0;

}

static void draw(unsigned int *buffer, unsigned int stride, unsigned int scale)
{

    unsigned int totw = WIDTH * scale;
    unsigned int lines = (snes->ppu->frameOverscan) ? 239 : 224;
    unsigned int top = (snes->ppu->frameOverscan) ? 1 : 8;
    unsigned int line;

    for (line = 0; line < HEIGHT; line++)
    {

        unsigned int field = 2;
        unsigned int i;

        for (i = 0; i < scale; i++)
        {

            unsigned int *out = buffer + (line * scale + i) * stride;
            unsigned int y = line - top;
            unsigned int current = (i * 2) / scale;

            if (line < top || y >= lines)
            {

                unsigned int x;

                for (x = 0; x < totw; x++)
                    out[x] = 0xFF000000;

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

}

static void present(void)
{

    unsigned int scale = getscale();
    unsigned int totw = WIDTH * scale;
    unsigned int toth = HEIGHT * scale;
    unsigned int offx = (fbwidth - totw) / 2;
    unsigned int offy = (fbheight - toth) / 2;
    struct message_videoflush videoflush;

    if (!fullscreen)
    {

        if (screen)
        {

            draw(screen, WIDTH, 1);
            flush();

        }

        return;

    }

    if (!framebuffer || !scale || totw > SCALEDWIDTH)
        return;

    draw(framebuffer + offy * fbwidth + offx, fbwidth, scale);

    videoflush.x = offx;
    videoflush.y = offy;
    videoflush.width = totw;
    videoflush.height = toth;

    channel_send(0, video, MESSAGE_TYPE_VIDEOFLUSH, sizeof (struct message_videoflush), &videoflush);

}

static unsigned int getframerate(void)
{

    return (snes->palTiming) ? 50 : 60;

}

static void sendaudio(unsigned int count)
{

    if (audioahead > (int)(rate / TICKRATE * AUDIOLIMIT))
        return;

    channel_send(1, audio, MESSAGE_TYPE_DATA, count * 4, audiobuffer);

    audioahead += count;

}

static void fillaudio(void)
{

    buffer_clear(audiobuffer, rate / TICKRATE * 4);

    while (audioahead < (int)(rate / TICKRATE * AUDIOCUSHION))
        sendaudio(rate / TICKRATE);

}

static void ontimertick(struct message *message)
{

    struct message_timertick *timertick = message_validate(MESSAGE_TYPE_TIMERTICK, message->data, message->length);
    unsigned int elapsed;

    if (timertick)
    {

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

}

static void setupaudio(void)
{

    audio = channel_lookup(option_getstring("audio-service"));
    rate = 48000;

}

static void setupvideo(void)
{

    struct message_videoconf videoconf;
    struct message_videoinfo videoinfo;

    videoconf.width = option_getdecimal("width");
    videoconf.height = option_getdecimal("height");
    videoconf.bpp = option_getdecimal("bpp");

    channel_send(0, video, MESSAGE_TYPE_VIDEOCONF, sizeof (struct message_videoconf), &videoconf);
    channel_send(1, video, MESSAGE_TYPE_INFO, 0, 0);

    if (channel_wait(1, video, MESSAGE_TYPE_VIDEOINFO, sizeof (struct message_videoinfo), &videoinfo) && videoinfo.bpp == 4)
    {

        framebuffer = (unsigned int *)videoinfo.framebuffer;
        fbwidth = videoinfo.width;
        fbheight = videoinfo.height;

        setupcolumns(getscale());

    }

}

static void releaseall(void)
{

    unsigned int i;

    for (i = 0; i < BUTTONS; i++)
        snes_setButtonState(snes, 1, i, 0);

    held = 0;

}

static void enterfullscreen(void)
{

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

    setupvideo();
    releaseall();

    keys.mod = 0;
    fullscreen = 1;
    wanted = 1;

    channel_send(0, keyboard, MESSAGE_TYPE_LINK, 0, 0);

}

static void leavefullscreen(void)
{

    channel_send(0, keyboard, MESSAGE_TYPE_UNLINK, 0, 0);
    releaseall();
    setupcolumns(1);

    framebuffer = 0;
    fullscreen = 0;
    wanted = 0;

    if (wm)
    {

        channel_send(1, wm, MESSAGE_TYPE_WMUNGRAB, 0, 0);
        channel_wait(1, wm, MESSAGE_TYPE_WMACK, 0, 0);

    }

}

static void switchscreen(unsigned int on)
{

    channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

    if (on)
        enterfullscreen();
    else
        leavefullscreen();

    channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);

}

static void run(void)
{

    while (channel_drain(0))
    {

        unsigned int due;

        if (wanted != fullscreen)
            switchscreen(wanted);

        due = ticks * getframerate() / TICKRATE;

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

static void press(unsigned int id, unsigned int mod)
{

    if (screen && (mod & KEYS_MOD_ALT) && (id == KEYS_KEY_F || id == KEYS_KEY_W))
    {

        wanted = (id == KEYS_KEY_F);

        return;

    }

    switch (id)
    {

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

static void release(unsigned int id)
{

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

static void onkeypress(struct message *message)
{

    struct message_keypress *keypress = message_validate(MESSAGE_TYPE_KEYPRESS, message->data, message->length);

    if (keypress)
    {

        unsigned int id = keys_getcode(&keys, keypress->scancode);

        press(id, keys.mod);

    }

}

static void onkeyrelease(struct message *message)
{

    struct message_keyrelease *keyrelease = message_validate(MESSAGE_TYPE_KEYRELEASE, message->data, message->length);

    if (keyrelease)
    {

        release(keys_getcode(&keys, keyrelease->scancode));

    }

}

static void onwmkeypress(struct message *message)
{

    struct message_wmkeypress *wmkeypress = message_validate(MESSAGE_TYPE_WMKEYPRESS, message->data, message->length);

    if (wmkeypress)
    {

        if (!fullscreen)
            press(wmkeypress->id, wmkeypress->keymod);

    }

}

static void onwmkeyrelease(struct message *message)
{

    struct message_wmkeyrelease *wmkeyrelease = message_validate(MESSAGE_TYPE_WMKEYRELEASE, message->data, message->length);

    if (wmkeyrelease)
    {

        if (!fullscreen)
            release(wmkeyrelease->id);

    }

}

static void onwmack(struct message *message)
{

    flushing = 0;

    if (dirty && !fullscreen)
        flush();

}

static unsigned int createscreen(void)
{

    unsigned int parent;
    unsigned int zero = 0;
    unsigned int i;

    screentarget = fs_auth("mem:");
    parent = (screentarget) ? fs_walk(1, screentarget, 0, "mem:") : 0;

    for (i = 0; i < SCREENS && parent && !screenid; i++)
    {

        cstring_write_fmt(screenname, 32, 0, "snesscreen%u\\0", &i);

        screenid = fs_create(1, screentarget, parent, screenname, cstring_length(screenname));

    }

    if (screenid && fs_write(1, screentarget, screenid, &zero, 4, SCREENSIZE - 4) == 4)
        screen = fs_mmap(1, screentarget, screenid, 0);

    return screen != 0;

}

static void destroyscreen(void)
{

    if (screen)
        fs_munmap(1, screentarget, screenid);

    if (screenid)
        fs_remove(1, screentarget, fs_walk(1, screentarget, 0, "mem:"), screenid);

}

static void showwindow(char *path)
{

    unsigned int width = WIDTH;
    unsigned int height = HEIGHT;

    setupcolumns(1);
    channel_send_fmt(0, wm, MESSAGE_TYPE_WMRENDERDATA, "+ window id \"window\" label \"SNES (%s)\" flow \"stretch\"\n+ media id \"screen\" in \"window\" mimetype \"image/raw\" source \"mem:%s\" width \"%u\" height \"%u\"\n", path + fs_dirlength(path), screenname, &width, &height);

}

static unsigned int load(unsigned int source, char *path)
{

    unsigned int target = fs_auth(path);
    unsigned int id = (target) ? fs_walk(1, target, 0, path) : 0;
    struct record record;
    unsigned char *data;

    if (!id || !fs_stat(1, target, id, &record))
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Path not found: %s\n", path);

        return 0;

    }

    data = malloc(record.size);
    snes = snes_init();

    if (!data || !snes || fs_read_full(1, target, id, data, record.size, 0) != record.size)
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Could not read: %s\n", path);

        return 0;

    }

    if (!snes_loadRom(snes, data, record.size))
    {

        channel_send_fmt(0, source, MESSAGE_TYPE_ERROR, "Not a supported SNES ROM: %s\n", path);

        return 0;

    }

    return 1;

}

static void onpath(struct message *message)
{

    if (started)
        return;

    started = 1;
    keyboard = channel_lookup(option_getstring("keyboard-service"));
    timer = channel_lookup(option_getstring("timer-service"));
    video = channel_lookup(option_getstring("video-service"));
    wm = channel_lookup(option_getstring("wm-service"));

    if (!keyboard || !timer || !video)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not find the keyboard, timer or video service\n");

        return;

    }

    if (!port_init())
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "No mem: file could be mapped\n");

        return;

    }

    setupaudio();

    if (load(message->source, message->data))
    {

        channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);
        channel_bind(MESSAGE_TYPE_KEYPRESS, onkeypress);
        channel_bind(MESSAGE_TYPE_KEYRELEASE, onkeyrelease);
        channel_bind(MESSAGE_TYPE_WMKEYPRESS, onwmkeypress);
        channel_bind(MESSAGE_TYPE_WMKEYRELEASE, onwmkeyrelease);
        channel_bind(MESSAGE_TYPE_WMACK, onwmack);

        if (wm && createscreen())
            showwindow(message->data);
        else
            enterfullscreen();

        channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);
        run();
        channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

        if (fullscreen)
            leavefullscreen();

        if (screen)
            channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);

    }

    destroyscreen();
    port_destroy();

}

void init(void)
{

    keys_init(&keys, KEYS_LAYOUT_QWERTY_US, (void *)0);
    option_add("width", "1024");
    option_add("height", "768");
    option_add("bpp", "4");
    option_add("audio-service", "mixer");
    option_add("keyboard-service", "keyboard:0");
    option_add("timer-service", "timer:0");
    option_add("video-service", "video:0");
    option_add("wm-service", "wm");
    channel_bind(MESSAGE_TYPE_PATH, onpath);

}

