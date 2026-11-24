#include <fudge.h>
#include <abi.h>

#define SCREENS                         16
#define WIDTH                           320
#define HEIGHT                          180
#define SCREENSIZE                      (WIDTH * HEIGHT * 4)
#define HORIZON                         140
#define SINES                           256
#define STARS                           48
#define BARS                            3
#define BARHEIGHT                       8
#define GLYPHS                          16
#define GLYPHSCALE                      3
#define GLYPHSIZE                       (8 * GLYPHSCALE)
#define TEXTY                           58
#define TEXTSWING                       30
#define RATE                            48000
#define TICKRATE                        60
#define FRAMES                          (RATE / TICKRATE)
#define VOICES                          4
#define WAVESIZE                        64
#define DRUMSIZE                        4800
#define HATSIZE                         1600
#define NOTES                           12
#define ROWS                            16
#define ROWTICKS                        7
#define CHORDS                          4
#define MELODYBARS                      8
#define INTROBARS                       4
#define LOOPBAR                         4
#define LEADBAR                         8
#define SONGBARS                        16
#define NOTEOFF                         1

struct glyph
{

    char c;
    unsigned char rows[8];

};

struct instrument
{

    signed char *sample;
    unsigned int length;
    unsigned int loop;
    int volume;
    int floor;
    int decay;
    unsigned int vibrato;

};

struct voice
{

    struct instrument *instrument;
    unsigned int position;
    unsigned int step;
    unsigned int note;
    unsigned int third;
    unsigned int age;
    int volume;

};

struct chord
{

    unsigned int root;
    unsigned int third;

};

static struct glyph glyphs[GLYPHS] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {'A', {0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}},
    {'D', {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}},
    {'E', {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00}},
    {'F', {0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00}},
    {'G', {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}},
    {'I', {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}},
    {'M', {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}},
    {'N', {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00}},
    {'O', {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}},
    {'P', {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}},
    {'R', {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00}},
    {'S', {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00}},
    {'T', {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}},
    {'U', {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}},
    {'Y', {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00}}
};

static char *text = "FUDGE OPERATING SYSTEM     ";
static unsigned int steps[NOTES] = {182890, 193765, 205287, 217494, 230426, 244128, 258645, 274025, 290319, 307582, 325872, 345249};
static struct chord chords[CHORDS] = {{45, 3}, {41, 4}, {48, 4}, {43, 4}};
static char *drumrow = "K.h.S.h.K.KhS.hh";
static char *bassrow = "0.c.0.c.0.c.07c7";
static char *arprow = "x..x..x.x..x..x.";
static unsigned char melody[MELODYBARS][ROWS] = {
    {76, 0, 0, 0, 69, 0, 72, 0, 76, 0, 0, 74, 72, 0, 71, 0},
    {72, 0, 74, 0, 72, 0, 69, 0, 65, 0, 0, 0, 69, 0, 0, 0},
    {67, 0, 72, 0, 76, 0, 0, 0, 79, 0, 76, 0, 72, 0, 74, 0},
    {74, 0, 0, 0, 71, 0, 67, 0, 71, 0, 74, 0, 79, 0, 0, 0},
    {81, 0, 0, 0, 79, 0, 76, 0, 81, 0, 0, 79, 76, 0, 72, 0},
    {77, 0, 0, 0, 76, 0, 72, 0, 69, 0, 72, 0, 77, 0, 76, 0},
    {76, 0, 0, 0, 72, 0, 67, 0, 72, 0, 76, 0, 79, 0, 0, 0},
    {74, 0, 0, 0, 71, 0, 74, 0, 79, 0, 0, 0, NOTEOFF, 0, 0, 0}
};
static signed char pulse[WAVESIZE];
static signed char saw[WAVESIZE];
static signed char square[WAVESIZE];
static signed char kick[DRUMSIZE];
static signed char snare[DRUMSIZE];
static signed char hat[HATSIZE];
static struct instrument kickinstrument = {kick, DRUMSIZE, 0, 64, 64, 0, 0};
static struct instrument snareinstrument = {snare, DRUMSIZE, 0, 48, 48, 0, 0};
static struct instrument hatinstrument = {hat, HATSIZE, 0, 24, 24, 0, 0};
static struct instrument bassinstrument = {saw, WAVESIZE, 1, 52, 20, 2, 0};
static struct instrument arpinstrument = {square, WAVESIZE, 1, 26, 8, 1, 0};
static struct instrument leadinstrument = {pulse, WAVESIZE, 1, 44, 30, 1, 1};
static struct voice voices[VOICES];
static int sine[SINES];
static unsigned int palette[SINES];
static unsigned int starx[STARS];
static unsigned int stary[STARS];
static unsigned int frame[WIDTH * HEIGHT];
static unsigned char sounddata[FRAMES * 4];
static unsigned int songtick;
static unsigned int songrow;
static unsigned int songbar;
static unsigned int ticks;
static unsigned int audio;
static unsigned int wm;
static unsigned int flushing;
static unsigned int dirty;
static unsigned int screentarget;
static unsigned int screenid;
static unsigned int *screen;
static char screenname[32];

static int computesine(unsigned int i)
{

    int x = i & 127;
    int p = x * (128 - x);
    int value = 1024 * p / (20480 - p);

    return (i & 128) ? -value : value;

}

static unsigned int lfsr(unsigned int value)
{

    return (value >> 1) ^ ((value & 1) ? 0xB400 : 0);

}

static unsigned int scale(unsigned int color, unsigned int level)
{

    unsigned int r = ((color >> 16) & 0xFF) * level / 255;
    unsigned int g = ((color >> 8) & 0xFF) * level / 255;
    unsigned int b = (color & 0xFF) * level / 255;

    return 0xFF000000 | (r << 16) | (g << 8) | b;

}

static void setupgraphics(void)
{

    unsigned int seed = 0xACE1;
    unsigned int i;

    for (i = 0; i < SINES; i++)
        sine[i] = computesine(i);

    for (i = 0; i < SINES; i++)
    {

        unsigned int r = (256 + sine[i]) / 2;
        unsigned int g = (256 + sine[(i + 85) % SINES]) / 2;
        unsigned int b = (256 + sine[(i + 170) % SINES]) / 2;

        palette[i] = 0xFF000000 | (((r > 255) ? 255 : r) << 16) | (((g > 255) ? 255 : g) << 8) | ((b > 255) ? 255 : b);

    }

    for (i = 0; i < STARS; i++)
    {

        seed = lfsr(seed);
        starx[i] = seed % WIDTH;
        seed = lfsr(seed);
        stary[i] = seed % HORIZON;

    }

}

static void drawbackground(void)
{

    unsigned int y;

    for (y = 0; y < HEIGHT; y++)
    {

        unsigned int color = 0xFF000000 | (y * 0x30 / HEIGHT);
        unsigned int x;

        for (x = 0; x < WIDTH; x++)
            frame[y * WIDTH + x] = color;

    }

}

static void drawstars(unsigned int t)
{

    unsigned int i;

    for (i = 0; i < STARS; i++)
    {

        unsigned int speed = i % 3 + 1;
        unsigned int x = (starx[i] + WIDTH - (t * speed) % WIDTH) % WIDTH;

        frame[stary[i] * WIDTH + x] = scale(0xFFFFFF, 0x50 * speed);

    }

}

static void drawbars(unsigned int t)
{

    unsigned int colors[BARS] = {0xFF4020, 0x40FF40, 0x4080FF};
    unsigned int i;

    for (i = 0; i < BARS; i++)
    {

        int center = HORIZON / 2 + sine[(t * 2 + i * 40) % SINES] * 52 / 256;
        int d;

        for (d = -BARHEIGHT; d <= BARHEIGHT; d++)
        {

            unsigned int color = scale(colors[i], 255 - ((d < 0) ? -d : d) * 255 / (BARHEIGHT + 1));
            unsigned int x;

            for (x = 0; x < WIDTH; x++)
                frame[(center + d) * WIDTH + x] = color;

        }

    }

}

static unsigned int findglyph(char c)
{

    unsigned int i;

    for (i = 0; i < GLYPHS; i++)
    {

        if (glyphs[i].c == c)
            return i;

    }

    return 0;

}

static void drawtext(unsigned int t, unsigned int offset, unsigned int shadow)
{

    unsigned int length = cstring_length(text) * GLYPHSIZE;
    unsigned int x;

    for (x = 0; x + offset < WIDTH; x++)
    {

        unsigned int position = (t * 2 + x) % length;
        struct glyph *glyph = &glyphs[findglyph(text[position / GLYPHSIZE])];
        unsigned int bit = 0x80 >> (position % GLYPHSIZE / GLYPHSCALE);
        unsigned int top = TEXTY + sine[(x + t * 3) % SINES] * TEXTSWING / 256 + offset;
        unsigned int r;

        for (r = 0; r < GLYPHSIZE; r++)
        {

            if (glyph->rows[r / GLYPHSCALE] & bit)
                frame[(top + r) * WIDTH + x + offset] = (shadow) ? 0xFF000010 : palette[(r * 6 + t * 4) % SINES];

        }

    }

}

static void drawmirror(unsigned int t)
{

    unsigned int y;

    for (y = HORIZON; y < HEIGHT; y++)
    {

        unsigned int k = y - HORIZON;
        unsigned int source = HORIZON - 1 - k * 2;
        int wobble = sine[(k * 10 + t * 6) % SINES] * 3 / 256;
        unsigned int x;

        for (x = 0; x < WIDTH; x++)
        {

            unsigned int color = frame[source * WIDTH + (x + WIDTH + wobble) % WIDTH];

            frame[y * WIDTH + x] = 0xFF000000 | (((color >> 1) & 0x7F7F7F) + 0x081830);

        }

    }

    for (y = 0; y < WIDTH; y++)
        frame[HORIZON * WIDTH + y] = palette[(y + t * 4) % SINES];

}

static void render(void)
{

    char *id = "screen";

    drawbackground();
    drawstars(ticks);
    drawbars(ticks);
    drawtext(ticks, 3, 1);
    drawtext(ticks, 0, 0);
    drawmirror(ticks);
    buffer_copy(screen, frame, SCREENSIZE);
    channel_sendid(0, wm, MESSAGE_TYPE_WMFLUSH, id);

    flushing = 1;
    dirty = 0;

}

static void setupsound(void)
{

    unsigned int seed = 0xBEEF;
    unsigned int phase = 0;
    unsigned int i;

    for (i = 0; i < WAVESIZE; i++)
    {

        pulse[i] = (i < WAVESIZE / 4) ? 96 : -96;
        saw[i] = -128 + i * 256 / WAVESIZE;
        square[i] = (i < WAVESIZE / 2) ? 80 : -80;

    }

    for (i = 0; i < DRUMSIZE; i++)
    {

        unsigned int left = DRUMSIZE - i;
        unsigned int frequency = 45 + 160 * (left / 48) * (left / 48) / ((DRUMSIZE / 48) * (DRUMSIZE / 48));

        phase += frequency * SINES * 256 / RATE;
        seed = lfsr(seed);
        kick[i] = sine[(phase >> 8) % SINES] * 127 / 256 * (int)left / DRUMSIZE;
        snare[i] = ((int)(seed & 0xFF) - 128) * 3 / 4 * (int)(left / 48) * (int)(left / 48) / ((DRUMSIZE / 48) * (DRUMSIZE / 48)) + sine[(i * 180 * SINES / RATE) % SINES] * (int)left / DRUMSIZE / 8;

    }

    for (i = 0; i < HATSIZE; i++)
    {

        unsigned int previous = seed;

        seed = lfsr(seed);
        hat[i] = ((int)(seed & 0xFF) - (int)(previous & 0xFF)) / 2 * (int)(HATSIZE - i) / HATSIZE;

    }

}

static unsigned int notestep(unsigned int note)
{

    return (note < 108) ? steps[note % NOTES] >> (8 - note / NOTES) : 0;

}

static void trigger(struct voice *voice, struct instrument *instrument, unsigned int note, unsigned int third)
{

    voice->instrument = instrument;
    voice->position = 0;
    voice->note = note;
    voice->third = third;
    voice->age = 0;
    voice->volume = instrument->volume;
    voice->step = (instrument->loop) ? notestep(note) : 0x10000;

}

static void playrow(void)
{

    struct chord *chord = &chords[songbar % CHORDS];

    switch (drumrow[songrow])
    {

    case 'K':
        trigger(&voices[0], &kickinstrument, 0, 0);

        break;

    case 'S':
        trigger(&voices[0], &snareinstrument, 0, 0);

        break;

    case 'h':
        trigger(&voices[0], &hatinstrument, 0, 0);

        break;

    }

    switch (bassrow[songrow])
    {

    case '0':
        trigger(&voices[1], &bassinstrument, chord->root - 12, 0);

        break;

    case '7':
        trigger(&voices[1], &bassinstrument, chord->root - 5, 0);

        break;

    case 'c':
        trigger(&voices[1], &bassinstrument, chord->root, 0);

        break;

    }

    if (songbar >= INTROBARS && arprow[songrow] == 'x')
        trigger(&voices[2], &arpinstrument, chord->root + 12, chord->third);

    if (songbar >= LEADBAR)
    {

        unsigned int note = melody[(songbar - LEADBAR) % MELODYBARS][songrow];

        if (note == NOTEOFF)
            voices[3].volume = 0;
        else if (note)
            trigger(&voices[3], &leadinstrument, note, 0);

    }

    if (++songrow == ROWS)
    {

        songrow = 0;
        songbar = (songbar + 1 == SONGBARS) ? LOOPBAR : songbar + 1;

    }

}

static void updatevoice(struct voice *voice)
{

    struct instrument *instrument = voice->instrument;

    if (!instrument)
        return;

    if (voice->age && voice->volume > instrument->floor)
        voice->volume = (voice->volume - instrument->decay > instrument->floor) ? voice->volume - instrument->decay : instrument->floor;

    if (voice->third)
    {

        switch (voice->age % 3)
        {

        case 0:
            voice->step = notestep(voice->note);

            break;

        case 1:
            voice->step = notestep(voice->note + voice->third);

            break;

        case 2:
            voice->step = notestep(voice->note + 7);

            break;

        }

    }

    if (instrument->vibrato && voice->age > 8)
    {

        int base = notestep(voice->note);

        voice->step = base + (base >> 6) * sine[(voice->age * 20) % SINES] / 256;

    }

    voice->age++;

}

static int mixvoice(struct voice *voice)
{

    struct instrument *instrument = voice->instrument;
    unsigned int index;

    if (!instrument)
        return 0;

    index = voice->position >> 16;

    if (index >= instrument->length)
    {

        if (!instrument->loop)
        {

            voice->instrument = 0;

            return 0;

        }

        voice->position -= instrument->length << 16;
        index -= instrument->length;

    }

    voice->position += voice->step;

    return instrument->sample[index] * voice->volume;

}

static int clip(int value)
{

    if (value > 32767)
        return 32767;

    if (value < -32767)
        return -32767;

    return value;

}

static void playsound(void)
{

    unsigned int i;

    if (!songtick)
        playrow();

    songtick = (songtick + 1) % ROWTICKS;

    for (i = 0; i < VOICES; i++)
        updatevoice(&voices[i]);

    for (i = 0; i < FRAMES; i++)
    {

        int a = mixvoice(&voices[0]) + mixvoice(&voices[3]);
        int b = mixvoice(&voices[1]) + mixvoice(&voices[2]);
        int left = clip((a * 3 + b) / 2);
        int right = clip((a + b * 3) / 2);

        sounddata[i * 4] = left;
        sounddata[i * 4 + 1] = left >> 8;
        sounddata[i * 4 + 2] = right;
        sounddata[i * 4 + 3] = right >> 8;

    }

    channel_send(1, audio, MESSAGE_TYPE_DATA, FRAMES * 4, sounddata);

}

static void ontimertick(struct message *message)
{

    ticks++;

    if (audio)
        playsound();

    if (flushing)
        dirty = 1;
    else
        render();

}

static void onwmack(struct message *message)
{

    flushing = 0;

    if (dirty)
        render();

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

        cstring_write_fmt(screenname, 32, 0, "aboutscreen%u\\0", &i);

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

static void setupaudio(void)
{

    unsigned int i;

    audio = channel_lookup(option_getstring("audio-service"));

    if (!audio)
        return;

    /* a few ticks of silence first, so the device has something to play while the next tick is made */
    buffer_clear(sounddata, FRAMES * 4);

    for (i = 0; i < 3; i++)
        channel_send(1, audio, MESSAGE_TYPE_DATA, FRAMES * 4, sounddata);

}

static void onmain(struct message *message)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));

    wm = channel_lookup(option_getstring("wm-service"));

    if (wm)
    {

        char *alfi = "initrd:data/alfi/wabout.alfi";
        unsigned int width = WIDTH;
        unsigned int height = HEIGHT;

        channel_send(0, wm, MESSAGE_TYPE_WMRENDERFILE, cstring_length_zero(alfi), alfi);

        if (timer && createscreen())
        {

            setupgraphics();
            setupsound();
            setupaudio();
            channel_send_fmt(0, wm, MESSAGE_TYPE_DATA, "= screen source \"mem:%s\" width \"%u\" height \"%u\"\n", screenname, &width, &height);
            channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);

        }

        channel_hold(0);

        if (screen)
            channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

        channel_send(0, wm, MESSAGE_TYPE_WMUNMAP, 0, 0);
        destroyscreen();

    }

}

void init(void)
{

    option_add("audio-service", "mixer");
    option_add("timer-service", "timer:0");
    option_add("wm-service", "wm");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);
    channel_bind(MESSAGE_TYPE_WMACK, onwmack);

}

