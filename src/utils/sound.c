#include <fudge.h>
#include <abi.h>

#define FRAMESIZE                       4

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

struct wavformat
{

    unsigned short format;
    unsigned short channels;
    unsigned int rate;
    unsigned int byterate;
    unsigned short align;
    unsigned short bits;

};

static unsigned int audio;
static char frames[MESSAGE_SIZE + FRAMESIZE];
static unsigned int framecount;

static void senddata(unsigned int source, void *buffer, unsigned int count)
{

    channel_send(1, audio, EVENT_DATA, count, buffer);

}

static unsigned int setformat(unsigned int source, unsigned int rate, unsigned int channels, unsigned int bits)
{

    struct event_audioconf audioconf;

    audioconf.rate = rate;
    audioconf.channels = channels;
    audioconf.bits = bits;

    if (channel_send(1, audio, EVENT_AUDIOCONF, sizeof (struct event_audioconf), &audioconf))
        return 1;

    channel_send_fmt(0, source, EVENT_ERROR, "Format not supported: %u Hz, %u channels, %u bits\n", &rate, &channels, &bits);

    return 0;

}

static void playwav(unsigned int source, char *path, unsigned int target, unsigned int id)
{

    struct riff riff;
    struct chunk chunk;
    struct wavformat format;
    char buffer[MESSAGE_SIZE];
    unsigned int offset = sizeof (struct riff);
    unsigned int found = 0;

    if (fs_read_full(1, target, id, &riff, sizeof (struct riff), 0) != sizeof (struct riff) || !buffer_match(riff.id, "RIFF", 4) || !buffer_match(riff.format, "WAVE", 4))
    {

        channel_send_fmt(0, source, EVENT_ERROR, "Not a WAV file: %s\n", path);

        return;

    }

    while (fs_read_full(1, target, id, &chunk, sizeof (struct chunk), offset) == sizeof (struct chunk))
    {

        offset += sizeof (struct chunk);

        if (buffer_match(chunk.id, "fmt ", 4) && chunk.size >= sizeof (struct wavformat))
        {

            if (fs_read_full(1, target, id, &format, sizeof (struct wavformat), offset) != sizeof (struct wavformat))
                break;

            found = 1;

        }

        if (buffer_match(chunk.id, "data", 4))
        {

            unsigned int end = offset + chunk.size;
            unsigned int count;

            if (!found || format.format != 1)
            {

                channel_send_fmt(0, source, EVENT_ERROR, "Not a PCM WAV file: %s\n", path);

                return;

            }

            if (!setformat(source, format.rate, format.channels, format.bits))
                return;

            while (offset < end && (count = fs_read(1, target, id, buffer, (end - offset < MESSAGE_SIZE) ? end - offset : MESSAGE_SIZE, offset)))
            {

                senddata(source, buffer, count);

                offset += count;

            }

            return;

        }

        offset += (chunk.size + 1) & ~1;

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
            senddata(message->source, frames, count);

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

