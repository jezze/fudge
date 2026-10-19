#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include "virtio.h"

#define BPP                             4
#define FRAMEBUFFER                     0xA0000000
#define RESOURCE                        1
#define POLLS                           0x1000000
#define CMD_RESOURCECREATE2D            0x0101
#define CMD_RESOURCEUNREF               0x0102
#define CMD_SETSCANOUT                  0x0103
#define CMD_RESOURCEFLUSH               0x0104
#define CMD_TRANSFERTOHOST2D            0x0105
#define CMD_RESOURCEATTACHBACKING       0x0106
#define RESPONSE_OKNODATA               0x1100
#define FORMAT_B8G8R8X8                 2
#define FORMAT_X8R8G8B8                 4

struct header
{

    unsigned int type;
    unsigned int flags;
    unsigned int fencelow;
    unsigned int fencehigh;
    unsigned int context;
    unsigned int padding;

};

struct rect
{

    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

};

struct create
{

    struct header header;
    unsigned int resource;
    unsigned int format;
    unsigned int width;
    unsigned int height;

};

struct unref
{

    struct header header;
    unsigned int resource;
    unsigned int padding;

};

struct attach
{

    struct header header;
    unsigned int resource;
    unsigned int count;
    unsigned int addresslow;
    unsigned int addresshigh;
    unsigned int length;
    unsigned int padding;

};

struct scanout
{

    struct header header;
    struct rect rect;
    unsigned int scanout;
    unsigned int resource;

};

struct transfer
{

    struct header header;
    struct rect rect;
    unsigned int offsetlow;
    unsigned int offsethigh;
    unsigned int resource;
    unsigned int padding;

};

struct flush
{

    struct header header;
    struct rect rect;
    unsigned int resource;
    unsigned int padding;

};

static struct base_driver driver;
static struct video_interface videointerface;
static struct spinlock spinlock;
static struct virtio_queue queue;
static unsigned char memory[VIRTIO_QUEUEMEMORY];
static struct virtio_device device;
static struct header response;
static unsigned int ready;
static unsigned int created;

static void setheader(struct header *header, unsigned int type)
{

    buffer_clear(header, sizeof (struct header));

    header->type = virtio_le32(type);

}

static void setrect(struct rect *rect, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    rect->x = virtio_le32(x);
    rect->y = virtio_le32(y);
    rect->width = virtio_le32(width);
    rect->height = virtio_le32(height);

}

/* one request at a time on the control queue, waiting for the device's answer */
static unsigned int command(void *request, unsigned int count)
{

    unsigned int length;
    unsigned int i;

    response.type = 0;

    virtio_setdesc(&queue, 0, request, count, VIRTIO_DESC_NEXT, 1);
    virtio_setdesc(&queue, 1, &response, sizeof (struct header), VIRTIO_DESC_WRITE, 0);
    virtio_push(&queue, 0);
    virtio_notify(&queue);

    for (i = 0; i < POLLS && !virtio_pending(&queue); i++);

    if (!virtio_pending(&queue))
        return 0;

    virtio_pop(&queue, &length);

    return virtio_le32(response.type) == RESPONSE_OKNODATA;

}

static void unref(void)
{

    struct unref request;

    setheader(&request.header, CMD_RESOURCEUNREF);

    request.resource = virtio_le32(RESOURCE);
    request.padding = 0;

    command(&request, sizeof (struct unref));

}

/* the wm writes 0xAARRGGBB words, whose bytes are B G R X on little-endian cpus and X R G B on big-endian ones */
static unsigned int create(unsigned int width, unsigned int height)
{

    struct create request;

    setheader(&request.header, CMD_RESOURCECREATE2D);

    request.resource = virtio_le32(RESOURCE);
    request.format = virtio_le32((virtio_le32(1) == 1) ? FORMAT_B8G8R8X8 : FORMAT_X8R8G8B8);
    request.width = virtio_le32(width);
    request.height = virtio_le32(height);

    return command(&request, sizeof (struct create));

}

static unsigned int attach(unsigned long address, unsigned int length)
{

    struct attach request;

    setheader(&request.header, CMD_RESOURCEATTACHBACKING);

    request.resource = virtio_le32(RESOURCE);
    request.count = virtio_le32(1);
    request.addresslow = virtio_le32(address);
    request.addresshigh = virtio_le32((address >> 16) >> 16);
    request.length = virtio_le32(length);
    request.padding = 0;

    return command(&request, sizeof (struct attach));

}

static unsigned int setscanout(unsigned int width, unsigned int height)
{

    struct scanout request;

    setheader(&request.header, CMD_SETSCANOUT);
    setrect(&request.rect, 0, 0, width, height);

    request.scanout = 0;
    request.resource = virtio_le32(RESOURCE);

    return command(&request, sizeof (struct scanout));

}

static void transfer(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    struct transfer request;
    unsigned int offset = (y * videointerface.width + x) * BPP;

    setheader(&request.header, CMD_TRANSFERTOHOST2D);
    setrect(&request.rect, x, y, width, height);

    request.offsetlow = virtio_le32(offset);
    request.offsethigh = 0;
    request.resource = virtio_le32(RESOURCE);
    request.padding = 0;

    command(&request, sizeof (struct transfer));

}

static void flush(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    struct flush request;

    setheader(&request.header, CMD_RESOURCEFLUSH);
    setrect(&request.rect, x, y, width, height);

    request.resource = virtio_le32(RESOURCE);
    request.padding = 0;

    command(&request, sizeof (struct flush));

}

/* the host shows its own copy of a resource backed by the framebuffer, created anew for every mode */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (width * height * BPP > ARCH_FRAMEBUFFER_SIZE)
        height = ARCH_FRAMEBUFFER_SIZE / (width * BPP);

    spinlock_acquire(&spinlock);

    if (created)
        unref();

    created = create(width, height) && attach(ARCH_FRAMEBUFFER_BASE, width * height * BPP) && setscanout(width, height);

    spinlock_release(&spinlock);

    if (created)
    {

        videointerface.width = width;
        videointerface.height = height;
        videointerface.bpp = BPP;

        arch_kmap(ARCH_FRAMEBUFFER_BASE, FRAMEBUFFER, width * height * BPP, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

    }

}

static void videointerface_flush(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

    if (!created || x >= videointerface.width || y >= videointerface.height)
        return;

    if (width > videointerface.width - x)
        width = videointerface.width - x;

    if (height > videointerface.height - y)
        height = videointerface.height - y;

    spinlock_acquire(&spinlock);
    transfer(x, y, width, height);
    flush(x, y, width, height);
    spinlock_release(&spinlock);

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, FRAMEBUFFER, 0, videointerface_setmode);

    videointerface.flush = videointerface_flush;

}

static unsigned int driver_match(unsigned int id)
{

    return virtio_match(id, VIRTIO_TYPE_GPU);

}

static void driver_reset(unsigned int id)
{

    if (!virtio_setup(&device, id))
        return;

    virtio_negotiate(&device, 0);

    if (!virtio_setqueue(&queue, &device, 0, memory) || queue.size < 2)
        return;

    virtio_nointerrupt(&queue);

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    virtio_setstatus(&device, VIRTIO_STATUS_DRIVEROK);
    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    video_unregisterinterface(&videointerface);
    virtio_reset(&device);

    ready = 0;
    created = 0;

}

void module_init(void)
{

    base_initdriver(&driver, "virtio-gpu", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    virtio_registerdriver(&driver);

}

void module_unregister(void)
{

    virtio_unregisterdriver(&driver);

}

