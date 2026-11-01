#include <fudge.h>
#include <abi.h>

#define CLIENTS                         4
#define RINGSIZE                        0x10000
#define RATE                            48000
#define FRAMESIZE                       4
#define FRAMEMAX                        (MESSAGE_SIZE / FRAMESIZE)
#define CUSHION                         3

struct client
{

    unsigned int source;
    unsigned int waiting;
    struct ring ring;

};

struct pace
{

    unsigned int counter;
    unsigned int started;
    unsigned int rest;
    unsigned int ahead;

};

struct resampler
{

    unsigned int phase;
    int previous[2];

};

static struct client clients[CLIENTS];
static char buffers[CLIENTS][RINGSIZE];
static int sums[FRAMEMAX * 2];
static unsigned char samples[MESSAGE_SIZE];
static unsigned char output[MESSAGE_SIZE];
static struct pace pace;
static struct resampler resampler;
static unsigned int audio;
static unsigned int pending;
static unsigned int rate;
static unsigned int hz;

static struct client *getclient(unsigned int source)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
    {

        if (clients[i].source == source)
            return &clients[i];

    }

    return 0;

}

static struct client *findclient(unsigned int source)
{

    struct client *client = getclient(source);
    unsigned int i;

    if (client)
        return client;

    for (i = 0; i < CLIENTS; i++)
    {

        if (!clients[i].source)
        {

            clients[i].source = source;
            clients[i].waiting = 0;

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

static void sendinfo(unsigned int target)
{

    struct event_audioinfo audioinfo;

    audioinfo.rate = RATE;
    audioinfo.channels = 2;
    audioinfo.bits = 16;

    channel_send(0, target, EVENT_AUDIOINFO, sizeof (struct event_audioinfo), &audioinfo);

}

static void putframe(unsigned int frame, int left, int right)
{

    output[frame * FRAMESIZE] = left;
    output[frame * FRAMESIZE + 1] = left >> 8;
    output[frame * FRAMESIZE + 2] = right;
    output[frame * FRAMESIZE + 3] = right >> 8;

}

/* linear interpolation down to the device's rate; phase counts in steps where one mixed frame is rate long and one device frame RATE */
static unsigned int resample(unsigned int frames)
{

    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < frames; i++)
    {

        while (resampler.phase < rate)
        {

            int fraction = resampler.phase * 32768 / rate;

            putframe(count++, resampler.previous[0] + (sums[i * 2] - resampler.previous[0]) * fraction / 32768, resampler.previous[1] + (sums[i * 2 + 1] - resampler.previous[1]) * fraction / 32768);

            resampler.phase += RATE;

        }

        resampler.phase -= rate;
        resampler.previous[0] = sums[i * 2];
        resampler.previous[1] = sums[i * 2 + 1];

    }

    return count;

}

/* samples are 16-bit stereo, little-endian like the devices take them; a client whose ring runs dry gives up its slot, and one waiting for room hears when there is a message's worth; the bytes of output are returned */
static unsigned int mix(unsigned int frames)
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

        if (client->waiting && ring_avail(&client->ring) >= MESSAGE_SIZE)
        {

            sendinfo(client->source);

            client->waiting = 0;

        }

        if (ring_isempty(&client->ring))
            client->source = 0;

    }

    for (j = 0; j < frames * 2; j++)
    {

        if (sums[j] > 32767)
            sums[j] = 32767;

        if (sums[j] < -32768)
            sums[j] = -32768;

    }

    if (rate == RATE)
    {

        for (j = 0; j < frames; j++)
            putframe(j, sums[j * 2], sums[j * 2 + 1]);

        return frames * FRAMESIZE;

    }

    return resample(frames) * FRAMESIZE;

}

/* placed without retrying, so a full device queue never stalls the mailbox; the output stays pending until the device takes it */
static unsigned int flush(void)
{

    if (pending && call_place(0, audio, EVENT_DATA, pending, output) == MESSAGE_RETRY)
        return 0;

    pending = 0;

    return 1;

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

/* the answer waits until the client's ring has room for a message, so a sender that waits for it never overflows */
static void oninfo(struct message *message)
{

    struct client *client = getclient(message->source);

    if (client && ring_avail(&client->ring) < MESSAGE_SIZE)
        client->waiting = 1;
    else
        sendinfo(message->source);

}

/* everything is counted in RATE frames; the device is kept CUSHION ticks ahead of the timer while anyone plays, unless its queue is full, and after a stall of more than a second the backlog is dropped */
static void ontimertick(struct message *message)
{

    struct event_timertick *timertick = message->data;
    unsigned int elapsed = timertick->counter - pace.counter;
    unsigned int played;

    if (!pace.started || elapsed > hz)
        elapsed = 0;

    pace.counter = timertick->counter;
    pace.started = 1;
    pace.rest += elapsed * RATE;
    played = pace.rest / hz;
    pace.rest %= hz;
    pace.ahead = (played < pace.ahead) ? pace.ahead - played : 0;

    if (!flush())
        return;

    while (countclients() && pace.ahead < CUSHION * RATE / hz)
    {

        unsigned int frames = RATE / hz;

        if (frames > FRAMEMAX)
            frames = FRAMEMAX;

        pending = mix(frames);
        pace.ahead += frames;

        if (!flush())
            break;

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

    if (setformat(RATE))
        rate = RATE;
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

    if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Could not announce service: %s\n", option_getstring("service"));

        return;

    }

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

