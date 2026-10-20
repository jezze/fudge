#include <fudge.h>
#include <abi.h>

#define FRAMESIZE                       4
#define FMTSIZE                         40
#define CHANNELS                        8
#define TAG_PCM                         0x0001
#define TAG_FLOAT                       0x0003
#define TAG_ALAW                        0x0006
#define TAG_MULAW                       0x0007
#define TAG_EXTENSIBLE                  0xFFFE

struct riff
{

    char id[4];
    unsigned int size;
    char format[4];

};

struct chunk
{

    char id[4];
    unsigned int size;

};

struct wav
{

    unsigned int tag;
    unsigned int channels;
    unsigned int rate;
    unsigned int bits;
    unsigned int align;
    unsigned int size;

};

struct output
{

    unsigned int rate;
    unsigned int channels;
    unsigned int framesize;
    unsigned int count;
    unsigned char buffer[MESSAGE_SIZE];

};

struct resampler
{

    unsigned int from;
    unsigned int to;
    unsigned int phase;
    unsigned int started;
    int previous[CHANNELS];
    int current[CHANNELS];
    int mixed[CHANNELS];

};

static unsigned int audio;
static char frames[MESSAGE_SIZE + FRAMESIZE];
static unsigned int framecount;

/* a WAV file's numbers are little-endian */
static unsigned int read16(void *data)
{

    unsigned char *bytes = data;

    return (bytes[1] << 8) | bytes[0];

}

static unsigned int read32(void *data)
{

    unsigned char *bytes = data;

    return ((unsigned int)bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];

}

static int decodefloat(void *data, unsigned int size)
{

    unsigned char *bytes = data;
    unsigned int high = read32(bytes + size - 4);
    unsigned int exponent = (size == 8) ? (high >> 20) & 0x7FF : (high >> 23) & 0xFF;
    unsigned int mantissa = (size == 8) ? (high & 0xFFFFF) | 0x100000 : (high & 0x7FFFFF) | 0x800000;
    unsigned int one = (size == 8) ? 1023 : 127;
    unsigned int shift = (size == 8) ? 1028 - exponent : 135 - exponent;
    int value;

    /* 1.0 and beyond, infinities and NaN clip; the shift turns the mantissa into the value times 32768 */
    if (exponent >= one)
        value = 32767;
    else if (shift >= 32)
        value = 0;
    else
        value = mantissa >> shift;

    return (high & 0x80000000) ? -value : value;

}

static int decodealaw(unsigned int value)
{

    unsigned int segment;
    int sample;

    value ^= 0x55;
    segment = (value & 0x70) >> 4;
    sample = (value & 0x0F) << 4;
    sample = (segment) ? (sample + 0x108) << (segment - 1) : sample + 8;

    return (value & 0x80) ? sample : -sample;

}

static int decodemulaw(unsigned int value)
{

    int sample;

    value = ~value & 0xFF;
    sample = (((value & 0x0F) << 3) + 0x84) << ((value & 0x70) >> 4);

    return (value & 0x80) ? 0x84 - sample : sample - 0x84;

}

/* one sample as signed 16 bits: integers keep their top two bytes */
static int decode(struct wav *wav, void *data)
{

    unsigned char *bytes = data;

    switch (wav->tag)
    {

    case TAG_FLOAT:
        return decodefloat(bytes, wav->size);

    case TAG_ALAW:
        return decodealaw(bytes[0]);

    case TAG_MULAW:
        return decodemulaw(bytes[0]);

    }

    if (wav->size == 1)
        return ((int)bytes[0] - 128) * 256;

    return (int)(read16(bytes + wav->size - 2) ^ 0x8000) - 0x8000;

}

static unsigned int checkwav(struct wav *wav)
{

    if (!wav->channels || !wav->rate || wav->align % wav->channels)
        return 0;

    wav->size = wav->align / wav->channels;

    switch (wav->tag)
    {

    case TAG_PCM:
        return wav->size >= 1 && wav->size <= 4;

    case TAG_FLOAT:
        return wav->size == 4 || wav->size == 8;

    case TAG_ALAW:
    case TAG_MULAW:
        return wav->size == 1;

    }

    return 0;

}

static unsigned int setformat(unsigned int rate, unsigned int channels, unsigned int bits)
{

    struct event_audioconf audioconf;

    audioconf.rate = rate;
    audioconf.channels = channels;
    audioconf.bits = bits;

    return channel_send(1, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf);

}

static void initoutput(struct output *output, unsigned int rate, unsigned int channels)
{

    output->rate = rate;
    output->channels = channels;
    output->framesize = channels * 2;
    output->count = 0;

}

/* what the device takes, closest first; anything but 8 or 16 bit integers is converted to 16 bits */
static unsigned int choose(struct wav *wav, struct output *output)
{

    if (wav->tag == TAG_PCM && wav->size <= 2 && setformat(wav->rate, wav->channels, wav->size * 8))
        return 1;

    if (wav->channels <= CHANNELS && setformat(wav->rate, wav->channels, 16))
        initoutput(output, wav->rate, wav->channels);
    else if (setformat(wav->rate, 2, 16))
        initoutput(output, wav->rate, 2);
    else if (setformat(48000, 2, 16))
        initoutput(output, 48000, 2);
    else if (setformat(44100, 2, 16))
        initoutput(output, 44100, 2);
    else
        return 0;

    return 2;

}

static void flush(struct output *output)
{

    if (output->count)
        channel_send(1, audio, EVENT_DATA, output->count, output->buffer);

    output->count = 0;

}

static void emit(struct output *output, struct resampler *resampler)
{

    unsigned int i;

    if (output->count + output->framesize > MESSAGE_SIZE)
        flush(output);

    for (i = 0; i < output->channels; i++)
    {

        unsigned int sample = resampler->mixed[i];

        output->buffer[output->count++] = sample;
        output->buffer[output->count++] = sample >> 8;

    }

}

/* a mono file plays on every channel; with fewer channels out, the first ones are kept */
static void decodeframe(struct wav *wav, struct resampler *resampler, unsigned int channels, void *data)
{

    unsigned char *bytes = data;
    unsigned int i;

    for (i = 0; i < channels; i++)
        resampler->current[i] = decode(wav, bytes + ((i < wav->channels) ? i : wav->channels - 1) * wav->size);

}

/* linear interpolation; phase counts in steps where one input frame is resampler->to long and one output frame resampler->from */
static void resample(struct output *output, struct resampler *resampler)
{

    unsigned int i;

    if (!resampler->started)
    {

        for (i = 0; i < output->channels; i++)
            resampler->previous[i] = resampler->current[i];

        resampler->started = 1;

    }

    while (resampler->phase < resampler->to)
    {

        int fraction = resampler->phase * 32768 / resampler->to;

        for (i = 0; i < output->channels; i++)
            resampler->mixed[i] = resampler->previous[i] + (resampler->current[i] - resampler->previous[i]) * fraction / 32768;

        emit(output, resampler);

        resampler->phase += resampler->from;

    }

    resampler->phase -= resampler->to;

    for (i = 0; i < output->channels; i++)
        resampler->previous[i] = resampler->current[i];

}

static void convert(struct wav *wav, struct output *output, struct resampler *resampler, void *data, unsigned int count)
{

    unsigned char *bytes = data;
    unsigned int offset;
    unsigned int i;

    for (offset = 0; offset + wav->align <= count; offset += wav->align)
    {

        decodeframe(wav, resampler, output->channels, bytes + offset);

        if (resampler->from == resampler->to)
        {

            for (i = 0; i < output->channels; i++)
                resampler->mixed[i] = resampler->current[i];

            emit(output, resampler);

        }

        else
        {

            resample(output, resampler);

        }

    }

}

static unsigned int readwav(struct wav *wav, unsigned int target, unsigned int id, unsigned int offset, unsigned int size)
{

    unsigned char fmt[FMTSIZE];
    unsigned int count = (size < FMTSIZE) ? size : FMTSIZE;

    if (size < 16 || fs_read_full(1, target, id, fmt, count, offset) != count)
        return 0;

    wav->tag = read16(fmt);
    wav->channels = read16(fmt + 2);
    wav->rate = read32(fmt + 4);
    wav->align = read16(fmt + 12);
    wav->bits = read16(fmt + 14);

    /* the subformat's first two bytes are the format tag it stands for */
    if (wav->tag == TAG_EXTENSIBLE && count >= FMTSIZE)
        wav->tag = read16(fmt + 24);

    return 1;

}

static void play(unsigned int source, struct wav *wav, unsigned int target, unsigned int id, unsigned int offset, unsigned int end)
{

    struct output output;
    struct resampler resampler;
    char buffer[MESSAGE_SIZE];
    unsigned int chunksize = MESSAGE_SIZE - MESSAGE_SIZE % wav->align;
    unsigned int passthrough;
    unsigned int count;

    passthrough = choose(wav, &output);

    if (!passthrough)
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Format not supported: %u Hz, %u channels, %u bits\n", &wav->rate, &wav->channels, &wav->bits);

        return;

    }

    resampler.from = wav->rate;
    resampler.to = (passthrough == 1) ? wav->rate : output.rate;
    resampler.phase = 0;
    resampler.started = 0;

    /* whole frames per message, since the drivers drop a partial one */
    while (offset < end && (count = fs_read_full(1, target, id, buffer, (end - offset < chunksize) ? end - offset : chunksize, offset)))
    {

        if (passthrough == 1)
            channel_send(1, audio, EVENT_DATA, count, buffer);
        else
            convert(wav, &output, &resampler, buffer, count);

        offset += count;

    }

    if (passthrough == 2)
        flush(&output);

}

static void playwav(unsigned int source, char *path, unsigned int target, unsigned int id)
{

    struct riff riff;
    struct chunk chunk;
    struct wav wav;
    unsigned int offset = sizeof (struct riff);
    unsigned int found = 0;

    if (fs_read_full(1, target, id, &riff, sizeof (struct riff), 0) != sizeof (struct riff) || !buffer_match(riff.id, "RIFF", 4) || !buffer_match(riff.format, "WAVE", 4))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Not a WAV file: %s\n", path);

        return;

    }

    while (fs_read_full(1, target, id, &chunk, sizeof (struct chunk), offset) == sizeof (struct chunk))
    {

        unsigned int size = read32(&chunk.size);

        offset += sizeof (struct chunk);

        if (buffer_match(chunk.id, "fmt ", 4))
            found = readwav(&wav, target, id, offset, size);

        if (buffer_match(chunk.id, "data", 4))
        {

            if (!found || !checkwav(&wav))
                channel_send_fmt(0, source, EVENT_ERROR, "Not a supported WAV file: %s\n", path);
            else
                play(source, &wav, target, id, offset, (size > 0xFFFFFFFF - offset) ? 0xFFFFFFFF : offset + size);

            return;

        }

        offset += (size + 1) & ~1;

    }

    channel_send_fmt(0, source, EVENT_ERROR, "No sound data in: %s\n", path);

}

static unsigned int lookup(unsigned int source)
{

    audio = channel_lookup(option_getstring("audio-service"));

    if (!audio)
        channel_send_fmt(0, source, EVENT_ERROR, "Audio service not found: %s\n", option_getstring("audio-service"));

    return audio;

}

static void ondata(struct message *message)
{

    if (audio || lookup(message->source))
    {

        unsigned int count;

        framecount += buffer_write(frames, MESSAGE_SIZE + FRAMESIZE, message->data, message->length, framecount);
        count = framecount - framecount % FRAMESIZE;

        if (count > MESSAGE_SIZE)
            count = MESSAGE_SIZE;

        if (count)
            channel_send(1, audio, EVENT_DATA, count, frames);

        buffer_copy(frames, frames + count, framecount - count);

        framecount -= count;

    }

}

static void onpath(struct message *message)
{

    unsigned int target = fs_auth(message->data);
    unsigned int id = (target) ? fs_walk(1, target, 0, message->data) : 0;

    if (!id)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Path not found: %s\n", message->data);

        return;

    }

    if (audio || lookup(message->source))
        playwav(message->source, message->data, target, id);

}

void init(void)
{

    option_add("audio-service", "audio");
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_PATH, onpath);

}

