#include <fudge.h>
#include <abi.h>

/* one second of the mixer's own format, so the sound card must play every frame unchanged; test/run checks what it played */
#define FRAMES                          48000
#define STEPS                           30000
#define FRAMESIZE                       4
#define HEADERSIZE                      44

static unsigned char chunk[PAGE_SIZE];

static void put16(unsigned int offset, unsigned int value)
{

    chunk[offset] = value;
    chunk[offset + 1] = value >> 8;

}

static void put32(unsigned int offset, unsigned int value)
{

    put16(offset, value);
    put16(offset + 2, value >> 16);

}

static void putheader(void)
{

    buffer_copy(chunk, "RIFF", 4);
    put32(4, HEADERSIZE - 8 + FRAMES * FRAMESIZE);
    buffer_copy(chunk + 8, "WAVEfmt ", 8);
    put32(16, 16);
    put16(20, 1);
    put16(22, 2);
    put32(24, 48000);
    put32(28, 48000 * FRAMESIZE);
    put16(32, FRAMESIZE);
    put16(34, 16);
    buffer_copy(chunk + 36, "data", 4);
    put32(40, FRAMES * FRAMESIZE);

}

static unsigned int writeramp(unsigned int target, unsigned int id)
{

    unsigned int offset = HEADERSIZE;
    unsigned int count = 0;
    unsigned int frame;

    putheader();

    if (fs_write_all(1, target, id, chunk, HEADERSIZE, 0) != HEADERSIZE)
        return 0;

    for (frame = 0; frame < FRAMES; frame++)
    {

        unsigned int value = frame % STEPS + 1;

        put16(count, value);
        put16(count + 2, 0x10000 - value);

        count += FRAMESIZE;

        if (count == PAGE_SIZE || frame == FRAMES - 1)
        {

            if (fs_write_all(1, target, id, chunk, count, offset) != count)
                return 0;

            offset += count;
            count = 0;

        }

    }

    return 1;

}

static void onmain(struct message *message)
{

    unsigned int target = fs_auth("mem:");
    unsigned int parent = (target) ? fs_walk(1, target, 0, "mem:") : 0;
    unsigned int id = (parent) ? fs_create(1, target, parent, "ramp.wav", 8) : 0;

    if (!channel_lookup("mixer"))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "no mixer, skipped\n");

        return;

    }

    if (!id || !writeramp(target, id))
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "mem:ramp.wav could not be written\n");

        return;

    }

    if (!system_run(message->source, "sound mem:ramp.wav"))
        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "sound did not exit cleanly\n");

}

void init(void)
{

    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

