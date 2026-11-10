#include <fudge.h>
#include <abi.h>

#define CLIENTS                         4
#define RINGSIZE                        0x10000
#define RATE                            48000
#define FRAMESIZE                       4
#define FRAMEMAX                        (MESSAGE_SIZE / FRAMESIZE)
#define CUSHION                         3
#define ROOT                            1
#define ROOTRECORDS                     2
#define DEVICES                         4
#define DEVICENAME                      32
#define VOLUME_FULL                     100

struct listing
{

    struct message_readresponse response;
    struct record records[ROOTRECORDS];

};

struct statresult
{

    struct message_statresponse response;
    struct record record;

};

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

struct device
{

    char name[DEVICENAME];
    unsigned int target;
    unsigned int volume;
    unsigned int rate;
    unsigned int pending;
    struct resampler resampler;
    unsigned char output[MESSAGE_SIZE];

};

struct devices
{

    struct device list[DEVICES];
    unsigned int count;

};

static struct client clients[CLIENTS];
static char buffers[CLIENTS][RINGSIZE];
static int sums[FRAMEMAX * 2];
static unsigned char samples[MESSAGE_SIZE];
static struct pace pace;
static struct devices devices;
static struct devices configured;
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

    if (!client)
    {

        unsigned int i;

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

    return client;

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

    struct message_audioinfo audioinfo;

    audioinfo.rate = RATE;
    audioinfo.channels = 2;
    audioinfo.bits = 16;

    channel_send(0, target, MESSAGE_TYPE_AUDIOINFO, sizeof (struct message_audioinfo), &audioinfo);

}

static void putframe(unsigned char *output, unsigned int frame, int left, int right)
{

    output[frame * FRAMESIZE] = left;
    output[frame * FRAMESIZE + 1] = left >> 8;
    output[frame * FRAMESIZE + 2] = right;
    output[frame * FRAMESIZE + 3] = right >> 8;

}

static int scale(int sample, unsigned int volume)
{

    sample = sample * (int)volume / VOLUME_FULL;

    if (sample > 32767)
        return 32767;

    if (sample < -32768)
        return -32768;

    return sample;

}

static unsigned int render(struct device *device, unsigned int frames)
{

    struct resampler *resampler = &device->resampler;
    unsigned int count = 0;
    unsigned int i;

    for (i = 0; i < frames; i++)
    {

        int left = scale(sums[i * 2], device->volume);
        int right = scale(sums[i * 2 + 1], device->volume);

        if (device->rate == RATE)
        {

            putframe(device->output, count++, left, right);

            continue;

        }

        while (resampler->phase < device->rate)
        {

            int fraction = resampler->phase * 32768 / device->rate;

            putframe(device->output, count++, resampler->previous[0] + (left - resampler->previous[0]) * fraction / 32768, resampler->previous[1] + (right - resampler->previous[1]) * fraction / 32768);

            resampler->phase += RATE;

        }

        resampler->phase -= device->rate;
        resampler->previous[0] = left;
        resampler->previous[1] = right;

    }

    return count * FRAMESIZE;

}

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

        if (client->waiting && ring_avail(&client->ring) >= MESSAGE_SIZE)
        {

            sendinfo(client->source);

            client->waiting = 0;

        }

        if (ring_isempty(&client->ring))
            client->source = 0;

    }

    for (i = 0; i < devices.count; i++)
        devices.list[i].pending = render(&devices.list[i], frames);

}

/* every device gets each mixed block before the next is mixed, so the slowest one paces all */
static unsigned int flush(void)
{

    unsigned int done = 1;
    unsigned int i;

    for (i = 0; i < devices.count; i++)
    {

        struct device *device = &devices.list[i];

        if (device->pending && call_place(0, device->target, MESSAGE_TYPE_DATA, device->pending, device->output) == MESSAGE_RETRY)
            done = 0;
        else
            device->pending = 0;

    }

    return done;

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

    struct client *client = getclient(message->source);

    if (client && ring_avail(&client->ring) < MESSAGE_SIZE)
        client->waiting = 1;
    else
        sendinfo(message->source);

}

static void ontimertick(struct message *message)
{

    struct message_timertick *timertick = message_validate(MESSAGE_TYPE_TIMERTICK, message->data, message->length);

    if (timertick)
    {

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

            mix(frames);

            pace.ahead += frames;

            if (!flush())
                break;

        }

    }

}

static unsigned int setformat(unsigned int target, unsigned int value)
{

    struct message_audioconf audioconf;

    audioconf.rate = value;
    audioconf.channels = 2;
    audioconf.bits = 16;

    return channel_send(0, target, MESSAGE_TYPE_AUDIOCONF, sizeof (struct message_audioconf), &audioconf);

}

static struct device *finddevice(unsigned int target)
{

    unsigned int i;

    for (i = 0; i < devices.count; i++)
    {

        if (devices.list[i].target == target)
            return &devices.list[i];

    }

    return 0;

}

static void configline(char *line, unsigned int length)
{

    unsigned int namelength = buffer_findbyte(line, length, ' ');

    if (namelength == 7 && length == 7 && buffer_match(line, "devices", 7))
    {

        configured.count = 0;

    }

    else if (namelength == 6 && buffer_match(line, "device", 6) && configured.count < DEVICES)
    {

        struct device *device = &configured.list[configured.count];
        char *rest = line + 7;
        unsigned int restlength = (length > 7) ? length - 7 : 0;
        unsigned int nameend = buffer_findbyte(rest, restlength, ' ');

        if (!nameend || nameend >= DEVICENAME)
            return;

        buffer_clear(device, sizeof (struct device));
        cstring_write_fmt(device->name, DEVICENAME, 0, "%w\\0", rest, &nameend);

        device->volume = (nameend < restlength) ? cstring_read_value(rest + nameend + 1, restlength - nameend - 1, 10) : VOLUME_FULL;

        if (device->volume > VOLUME_FULL)
            device->volume = VOLUME_FULL;

        configured.count++;

    }

}

static void configlines(char *data, unsigned int count)
{

    unsigned int length;
    char *line;

    for (line = data; line < data + count; line += length + 1)
    {

        length = buffer_findbyte(line, data + count - line, '\n');

        if (length)
            configline(line, length);

    }

}

/* devices already playing keep their format and position; new ones are configured once */
static void applyconfig(void)
{

    struct devices next;
    unsigned int i;

    next.count = 0;

    for (i = 0; i < configured.count; i++)
    {

        unsigned int target = channel_lookup(configured.list[i].name);
        struct device *old = (target) ? finddevice(target) : 0;
        struct device *device = &next.list[next.count];

        if (!target)
            continue;

        if (old)
        {

            buffer_copy(device, old, sizeof (struct device));

        }

        else
        {

            buffer_clear(device, sizeof (struct device));

            device->target = target;

            if (setformat(target, RATE))
                device->rate = RATE;
            else if (setformat(target, 44100))
                device->rate = 44100;

            if (!device->rate)
                continue;

        }

        buffer_copy(device->name, configured.list[i].name, DEVICENAME);

        device->volume = configured.list[i].volume;
        next.count++;

    }

    buffer_copy(&devices, &next, sizeof (struct devices));

}

static void onconfig(struct message *message)
{

    configlines(message->data, message->length);
    applyconfig();

}

static unsigned int walk(unsigned int parent, unsigned int length, char *path)
{

    if (!parent || parent == ROOT)
    {

        unsigned int offset = 0;

        while (offset < length)
        {

            char *cp = path + offset;
            unsigned int cl = buffer_findbyte(cp, length - offset, '/');

            if (cl > 2 || (cl && !buffer_match(cp, "..", cl)))
                return 0;

            offset += cl + 1;

        }

        return ROOT;

    }

    return 0;

}

static void onreadrequest(struct message *message)
{

    struct message_readrequest *request = message_validate(MESSAGE_TYPE_READREQUEST, message->data, message->length);
    struct listing listing;

    if (request)
    {

        unsigned int c = 0;
        unsigned int i;

        for (i = request->offset; request->id == ROOT && i < ROOTRECORDS && (c + 1) * sizeof (struct record) <= request->count; i++)
            record_init(&listing.records[c++], ROOT, RECORD_TYPE_DIRECTORY, 0, i + 1, i + 1, "..");

        listing.response.count = c * sizeof (struct record);
        listing.response.buffer = 0;

        channel_send(0, message->source, MESSAGE_TYPE_READRESPONSE, sizeof (struct message_readresponse) + listing.response.count, &listing);

    }

}

static void onstatrequest(struct message *message)
{

    struct message_statrequest *request = message_validate(MESSAGE_TYPE_STATREQUEST, message->data, message->length);
    struct statresult result;

    if (request)
    {

        record_init(&result.record, ROOT, RECORD_TYPE_DIRECTORY, 0, 1, 1, ".");

        result.response.count = (request->id == ROOT) ? sizeof (struct record) : 0;

        channel_send(0, message->source, MESSAGE_TYPE_STATRESPONSE, sizeof (struct message_statresponse) + result.response.count, &result);

    }

}

static void onwalkrequest(struct message *message)
{

    struct message_walkrequest *request = message_validate(MESSAGE_TYPE_WALKREQUEST, message->data, message->length);
    struct message_walkresponse response;

    if (request)
    {

        response.id = walk(request->parent, request->length, (char *)(request + 1));

        channel_send(0, message->source, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response);

    }

}

static void onmain(struct message *message)
{

    static char data[0x1000];
    unsigned int timer = channel_lookup(option_getstring("timer-service"));
    struct message_timerinfo timerinfo;

    configlines("devices", 7);
    configlines(data, system_feed(0, 0, data, 0x1000, "mq -query .devices -format \"device {device} {volume}\" %s", option_getstring("config")));
    applyconfig();

    if (!devices.count)
        return;

    if (!timer)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Timer service not found: %s\n", option_getstring("timer-service"));

        return;

    }

    channel_send(1, timer, MESSAGE_TYPE_INFO, 0, 0);

    if (!channel_wait(1, timer, MESSAGE_TYPE_TIMERINFO, sizeof (struct message_timerinfo), &timerinfo) || !timerinfo.hz)
        return;

    hz = timerinfo.hz;

    if (call_announce(0, cstring_length(option_getstring("service")), option_getstring("service")) != MESSAGE_OK)
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not announce service: %s\n", option_getstring("service"));

        return;

    }

    channel_send(0, timer, MESSAGE_TYPE_LINK, 0, 0);
    channel_hold(0);
    channel_send(0, timer, MESSAGE_TYPE_UNLINK, 0, 0);

}

void init(void)
{

    unsigned int i;

    for (i = 0; i < CLIENTS; i++)
        ring_init(&clients[i].ring, RINGSIZE, buffers[i]);

    option_add("service", "mixer");
    option_add("config", "initrd:data/config/audio.mq");
    option_add("timer-service", "timer:0");
    channel_bind(MESSAGE_TYPE_CONFIG, onconfig);
    channel_bind(MESSAGE_TYPE_DATA, ondata);
    channel_bind(MESSAGE_TYPE_INFO, oninfo);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);
    channel_bind(MESSAGE_TYPE_READREQUEST, onreadrequest);
    channel_bind(MESSAGE_TYPE_STATREQUEST, onstatrequest);
    channel_bind(MESSAGE_TYPE_TIMERTICK, ontimertick);
    channel_bind(MESSAGE_TYPE_WALKREQUEST, onwalkrequest);

}

