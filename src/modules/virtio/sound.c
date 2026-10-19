#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/audio.h>
#include "virtio.h"

#define STREAM                          0
#define BUFFERS                         16
#define BUFFERSIZE                      MESSAGE_SIZE
#define RATES                           14
#define POLLS                           0x1000000
#define QUEUE_CONTROL                   0
#define QUEUE_TX                        2
#define CONFIG_STREAMS                  4
#define CMD_PCMINFO                     0x0100
#define CMD_PCMSETPARAMS                0x0101
#define CMD_PCMPREPARE                  0x0102
#define CMD_PCMRELEASE                  0x0103
#define CMD_PCMSTART                    0x0104
#define CMD_PCMSTOP                     0x0105
#define STATUS_OK                       0x8000
#define DIRECTION_OUTPUT                0
#define FORMAT_U8                       4
#define FORMAT_S16                      5

struct header
{

    unsigned int code;

};

struct query
{

    struct header header;
    unsigned int start;
    unsigned int count;
    unsigned int size;

};

struct info
{

    unsigned int nid;
    unsigned int features;
    unsigned int formatslow;
    unsigned int formatshigh;
    unsigned int rateslow;
    unsigned int rateshigh;
    unsigned char direction;
    unsigned char channelsmin;
    unsigned char channelsmax;
    unsigned char padding[5];

};

struct pcm
{

    struct header header;
    unsigned int stream;

};

struct params
{

    struct header header;
    unsigned int stream;
    unsigned int bufferbytes;
    unsigned int periodbytes;
    unsigned int features;
    unsigned char channels;
    unsigned char format;
    unsigned char rate;
    unsigned char padding;

};

struct xfer
{

    unsigned int stream;

};

struct status
{

    unsigned int status;
    unsigned int latency;

};

static struct base_driver driver;
static struct audio_interface audiointerface;
static struct spinlock spinlock;
static struct virtio_queue controlqueue;
static struct virtio_queue txqueue;
static unsigned char controlmemory[VIRTIO_QUEUEMEMORY];
static unsigned char txmemory[VIRTIO_QUEUEMEMORY];
static unsigned char buffers[BUFFERS][BUFFERSIZE];
static struct status statuses[BUFFERS];
static struct xfer xfer;
static struct header response;
static struct info info;
static struct virtio_device device;
static unsigned int rates[RATES] = {5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000, 384000};
static unsigned int ready;
static unsigned int running;
static unsigned int framesize;
static unsigned int txnext;
static unsigned int txpending;

/* one request at a time on the control queue, waiting for the device's answer and any data after it */
static unsigned int command(void *request, unsigned int count, void *reply, unsigned int replycount)
{

    unsigned int length;
    unsigned int i;

    response.code = 0;

    virtio_setdesc(&controlqueue, 0, request, count, VIRTIO_DESC_NEXT, 1);
    virtio_setdesc(&controlqueue, 1, &response, sizeof (struct header), VIRTIO_DESC_WRITE | ((replycount) ? VIRTIO_DESC_NEXT : 0), 2);
    virtio_setdesc(&controlqueue, 2, reply, replycount, VIRTIO_DESC_WRITE, 0);
    virtio_push(&controlqueue, 0);
    virtio_notify(&controlqueue);

    for (i = 0; i < POLLS && !virtio_pending(&controlqueue); i++);

    if (!virtio_pending(&controlqueue))
        return 0;

    virtio_pop(&controlqueue, &length);

    return virtio_le32(response.code) == STATUS_OK;

}

static unsigned int pcmcommand(unsigned int code)
{

    struct pcm request;

    request.header.code = virtio_le32(code);
    request.stream = virtio_le32(STREAM);

    return command(&request, sizeof (struct pcm), 0, 0);

}

static unsigned int queryinfo(void)
{

    struct query request;

    request.header.code = virtio_le32(CMD_PCMINFO);
    request.start = virtio_le32(STREAM);
    request.count = virtio_le32(1);
    request.size = virtio_le32(sizeof (struct info));

    return command(&request, sizeof (struct query), &info, sizeof (struct info)) && info.direction == DIRECTION_OUTPUT;

}

static unsigned int setparams(unsigned int channels, unsigned int format, unsigned int rate)
{

    struct params request;

    request.header.code = virtio_le32(CMD_PCMSETPARAMS);
    request.stream = virtio_le32(STREAM);
    request.bufferbytes = virtio_le32(BUFFERS * BUFFERSIZE);
    request.periodbytes = virtio_le32(BUFFERSIZE);
    request.features = 0;
    request.channels = channels;
    request.format = format;
    request.rate = rate;
    request.padding = 0;

    return command(&request, sizeof (struct params), 0, 0);

}

static void reclaim(void)
{

    unsigned int length;

    while (virtio_pending(&txqueue))
    {

        virtio_pop(&txqueue, &length);

        txpending--;

    }

}

/* samples go to the device as they come: virtio wants them little-endian, like WAV */
static unsigned int audiointerface_setformat(unsigned int rate, unsigned int channels, unsigned int bits)
{

    unsigned int format = (bits == 8) ? FORMAT_U8 : FORMAT_S16;
    unsigned int index;

    if (bits != 8 && bits != 16)
        return 0;

    if (!(virtio_le32(info.formatslow) & (1 << format)))
        return 0;

    if (channels < info.channelsmin || channels > info.channelsmax)
        return 0;

    for (index = 0; index < RATES && rates[index] != rate; index++);

    if (index == RATES || !(virtio_le32(info.rateslow) & (1 << index)))
        return 0;

    spinlock_acquire(&spinlock);

    if (running)
    {

        pcmcommand(CMD_PCMSTOP);
        pcmcommand(CMD_PCMRELEASE);

    }

    running = setparams(channels, format, index) && pcmcommand(CMD_PCMPREPARE) && pcmcommand(CMD_PCMSTART);

    if (running)
    {

        audiointerface.rate = rate;
        audiointerface.channels = channels;
        audiointerface.bits = bits;
        framesize = channels * bits / 8;

    }

    spinlock_release(&spinlock);

    return running;

}

static unsigned int audiointerface_write(void *buffer, unsigned int count)
{

    unsigned int head;

    /* whole frames only */
    count -= count % framesize;

    if (!count)
        return 1;

    spinlock_acquire(&spinlock);
    reclaim();

    if (!running || txpending == BUFFERS)
    {

        spinlock_release(&spinlock);

        return !running;

    }

    head = txnext * 3;

    buffer_copy(buffers[txnext], buffer, count);
    virtio_setdesc(&txqueue, head, &xfer, sizeof (struct xfer), VIRTIO_DESC_NEXT, head + 1);
    virtio_setdesc(&txqueue, head + 1, buffers[txnext], count, VIRTIO_DESC_NEXT, head + 2);
    virtio_setdesc(&txqueue, head + 2, &statuses[txnext], sizeof (struct status), VIRTIO_DESC_WRITE, 0);
    virtio_push(&txqueue, head);
    virtio_notify(&txqueue);

    txnext = (txnext + 1) % BUFFERS;
    txpending++;

    spinlock_release(&spinlock);

    return 1;

}

static void driver_init(unsigned int id)
{

    audio_initinterface(&audiointerface, id, audiointerface_setformat, audiointerface_write);

}

static unsigned int driver_match(unsigned int id)
{

    return virtio_match(id, VIRTIO_TYPE_SOUND);

}

static void driver_reset(unsigned int id)
{

    if (!virtio_setup(&device, id))
        return;

    virtio_negotiate(&device, 0);

    if (!virtio_getconfig(&device, CONFIG_STREAMS))
        return;

    if (!virtio_setqueue(&controlqueue, &device, QUEUE_CONTROL, controlmemory) || controlqueue.size < 3)
        return;

    if (!virtio_setqueue(&txqueue, &device, QUEUE_TX, txmemory) || txqueue.size < BUFFERS * 3)
        return;

    virtio_nointerrupt(&controlqueue);
    virtio_nointerrupt(&txqueue);

    xfer.stream = virtio_le32(STREAM);
    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);

    if (queryinfo() && audiointerface_setformat(48000, 2, 16))
        audio_registerinterface(&audiointerface);
    else
        ready = 0;

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    audio_unregisterinterface(&audiointerface);
    virtio_reset(&device);

    ready = 0;
    running = 0;
    txpending = 0;
    txnext = 0;

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-sound", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    virtio_registerdriver(&driver);

}

void module_unregister(void)
{

    virtio_unregisterdriver(&driver);

}

