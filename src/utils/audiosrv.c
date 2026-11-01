#include <fudge.h>
#include <abi.h>

#define CLIENTS                         8
#define RINGSIZE                        0x8000
#define FRAMESIZE                       4
#define FRAMEMAX                        (MESSAGE_SIZE / FRAMESIZE)
#define CUSHION                         3

struct client
{

    unsigned int source;
    struct ring ring;

};

struct pace
{

    unsigned int counter;
    unsigned int started;
    unsigned int rest;
    unsigned int ahead;

};

static struct client clients[CLIENTS];
static char buffers[CLIENTS][RINGSIZE];
static int sums[FRAMEMAX * 2];
static unsigned char samples[MESSAGE_SIZE];
static unsigned char output[MESSAGE_SIZE];
static struct pace pace;
static unsigned int audio;
static unsigned int rate;
static unsigned int hz;

static struct client *findclient(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
    {

        if (clients[i].source == source)
            return &clients[i];

    }

    for (i = 0; i < CLIENTS; i++)
    {

        if (!clients[i].source)
        {

            clients[i].source = source;

            ring_reset(&clients[i].ring);

            return &clients[i];

        }

    }

    return 0;

}

static unsigned int countclients(void)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
    {

        if (clients[i].source)
            count++;

    }

    return count;

}

/* samples are 16-bit stereo, little-endian like the devices take them; a client whose ring runs dry gives up its slot */
static void mix(unsigned int frames)
{

    unsigned int i;
    unsigned int j;

    for (j = 0; j < frames * 2; j++)
        sums[j] = 0;

    for (i = 0; i < CLIENTS; i++)
    {

        struct client *client = &clients[i];
        unsigned int count;

        if (!client->source)
            continue;

        count = ring_read(&client->ring, samples, frames * FRAMESIZE);

        for (j = 0; j < count / 2; j++)
            sums[j] += (int)(((samples[j * 2 + 1] << 8) | samples[j * 2]) ^ 0x8000) - 0x8000;

        if (ring_isempty(&client->ring))
            client->source = 0;

    }

    for (j = 0; j < frames * 2; j++)
    {

        int sum = sums[j];

        if (sum > 32767)
            sum = 32767;

        if (sum < -32768)
            sum = -32768;

        output[j * 2] = sum;
        output[j * 2 + 1] = sum >> 8;

    }

    channel_send(0, audio, EVENT_DATA, frames * FRAMESIZE, output);

}

static void ondata(struct message *message)
{

    struct client *client = findclient(message->source);

    if (client)
    {

        unsigned int avail = ring_avail(&client->ring);
        unsigned int count = message->length - message->length % FRAMESIZE;

        ring_write(&client->ring, message->data, (count < avail) ? count : avail - avail % FRAMESIZE);

    }

}

static void oninfo(struct message *message)
{

    struct event_audioinfo audioinfo;

    audioinfo.rate = rate;
    audioinfo.channels = 2;
    audioinfo.bits = 16;

    channel_send(0, message->source, EVENT_AUDIOINFO, sizeof (struct event_audioinfo), &audioinfo);

}

/* the device is kept CUSHION ticks ahead of the timer while anyone plays; after a stall of more than a second the backlog is dropped */
static void ontimertick(struct message *message)
{

    struct event_timertick *timertick = message->data;
    unsigned int elapsed = timertick->counter - pace.counter;
    unsigned int played;

    if (!pace.started || elapsed > hz)
        elapsed = 0;

    pace.counter = timertick->counter;
    pace.started = 1;
    pace.rest += elapsed * rate;
    played = pace.rest / hz;
    pace.rest %= hz;
    pace.ahead = (played < pace.ahead) ? pace.ahead - played : 0;

    while (countclients() && pace.ahead < CUSHION * rate / hz)
    {

        unsigned int frames = rate / hz;

        if (frames > FRAMEMAX)
            frames = FRAMEMAX;

        mix(frames);

        pace.ahead += frames;

    }

}

static unsigned int setformat(unsigned int value)
{

    struct event_audioconf audioconf;

    audioconf.rate = value;
    audioconf.channels = 2;
    audioconf.bits = 16;

    return channel_send(0, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf);

}

static void onmain(struct message *message)
{

    unsigned int timer = channel_lookup(option_getstring("timer-service"));
    struct event_timerinfo timerinfo;

    audio = channel_lookup(option_getstring("audio-service"));

    if (!audio)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Audio service not found: %s\n", option_getstring("audio-service"));

        return;

    }

    if (!timer)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Timer service not found: %s\n", option_getstring("timer-service"));

        return;

    }

    if (setformat(48000))
        rate = 48000;
    else if (setformat(44100))
        rate = 44100;

    if (!rate)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Audio device takes neither 48000 nor 44100 Hz 16-bit stereo\n");

        return;

    }

    channel_send(1, timer, EVENT_INFO, 0, 0);

    if (!channel_wait(1, timer, EVENT_TIMERINFO, sizeof (struct event_timerinfo), &timerinfo) || !timerinfo.hz)
        return;

    hz = timerinfo.hz;

    call_announce(0, cstring_length(option_getstring("service")), option_getstring("service"));
    channel_send(0, timer, EVENT_LINK, 0, 0);
    channel_hold(0);
    channel_send(0, timer, EVENT_UNLINK, 0, 0);

}

void init(void)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
        ring_init(&clients[i].ring, RINGSIZE, buffers[i]);

    option_add("service", "mixer");
    option_add("audio-service", "audio");
    option_add("timer-service", "timer");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_INFO, oninfo);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_TIMERTICK, ontimertick);

}

