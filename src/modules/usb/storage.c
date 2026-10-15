#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/block.h>
#include "usb.h"

#define CLASS_STORAGE                   0x08
#define SUBCLASS_SCSI                   0x06
#define PROTOCOL_BULKONLY               0x50
#define CBW_SIGNATURE                   0x43425355
#define CSW_SIGNATURE                   0x53425355
#define SCSI_TESTUNITREADY              0x00
#define SCSI_REQUESTSENSE               0x03
#define SCSI_READCAPACITY               0x25
#define SCSI_READ                       0x28
#define SCSI_WRITE                      0x2A
#define TRANSFERSIZE                    0x4000
#define RETRIES                         8

struct cbw
{

    unsigned int signature;
    unsigned int tag;
    unsigned int length;
    unsigned char flags;
    unsigned char lun;
    unsigned char cblength;
    unsigned char cb[16];

} __attribute__((packed));

struct csw
{

    unsigned int signature;
    unsigned int tag;
    unsigned int residue;
    unsigned char status;

} __attribute__((packed));

static struct base_driver driver;
static struct block_interface blockinterface;
static struct spinlock spinlock;
static unsigned int tag;
static unsigned int ready;

static unsigned int command(unsigned int id, unsigned char *cb, unsigned int cblength, unsigned int in, void *data, unsigned int length)
{

    struct cbw cbw;
    struct csw csw;

    buffer_clear(&cbw, sizeof (struct cbw));

    cbw.signature = CBW_SIGNATURE;
    cbw.tag = ++tag;
    cbw.length = length;
    cbw.flags = (in) ? 0x80 : 0x00;
    cbw.cblength = cblength;

    buffer_copy(cbw.cb, cb, cblength);

    if (usb_bulk(id, 0, &cbw, sizeof (struct cbw)) != sizeof (struct cbw))
        return 0;

    if (length && usb_bulk(id, in, data, length) != length)
        return 0;

    if (usb_bulk(id, 1, &csw, sizeof (struct csw)) != sizeof (struct csw))
        return 0;

    return csw.signature == CSW_SIGNATURE && csw.tag == cbw.tag && csw.status == 0;

}

static unsigned int testready(unsigned int id)
{

    unsigned int i;

    for (i = 0; i < RETRIES; i++)
    {

        unsigned char cb[6] = {SCSI_TESTUNITREADY};
        unsigned char sense[18];
        unsigned char request[6] = {SCSI_REQUESTSENSE, 0, 0, 0, sizeof (sense), 0};

        if (command(id, cb, sizeof (cb), 0, 0, 0))
            return 1;

        /* a fresh device reports a unit attention first, which reading the sense data clears */
        command(id, request, sizeof (request), 1, sense, sizeof (sense));

    }

    return 0;

}

static unsigned int readcapacity(unsigned int id)
{

    unsigned char cb[10] = {SCSI_READCAPACITY};
    unsigned char data[8];

    if (!command(id, cb, sizeof (cb), 1, data, sizeof (data)))
        return 0;

    return (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

}

static unsigned int transfer(unsigned int id, unsigned int write, unsigned int block, void *data, unsigned int count)
{

    unsigned int blocks = count / blockinterface.blocksize;
    unsigned char cb[10];

    buffer_clear(cb, sizeof (cb));

    cb[0] = (write) ? SCSI_WRITE : SCSI_READ;
    cb[2] = block >> 24;
    cb[3] = block >> 16;
    cb[4] = block >> 8;
    cb[5] = block;
    cb[7] = blocks >> 8;
    cb[8] = blocks;

    return command(id, cb, sizeof (cb), !write, data, count);

}

static void blockinterface_startsession(struct block_session *session)
{

    unsigned char *buffer = (unsigned char *)kernel_getbuffer(session->source);
    unsigned int blocksize = blockinterface.blocksize;
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;
    unsigned int offset;

    count -= count % blocksize;

    spinlock_acquire(&spinlock);

    for (offset = 0; buffer && offset < count; offset += TRANSFERSIZE)
    {

        unsigned int length = (count - offset > TRANSFERSIZE) ? TRANSFERSIZE : count - offset;

        if (!transfer(blockinterface.id, session->type == BLOCK_TYPE_WRITE, (session->start + offset) / blocksize, buffer + offset, length))
            break;

    }

    session->count = (offset < count) ? offset : count;

    spinlock_release(&spinlock);
    block_session_done(&blockinterface, session);

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, 0, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return usb_getclass(id) == CLASS_STORAGE && usb_getsubclass(id) == SUBCLASS_SCSI && usb_getprotocol(id) == PROTOCOL_BULKONLY;

}

static void driver_reset(unsigned int id)
{

    ready = testready(id);

    if (ready)
        blockinterface.blocksize = readcapacity(id);

    ready = ready && blockinterface.blocksize;

}

static void driver_attach(unsigned int id)
{

    if (ready)
        block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (ready)
        block_unregisterinterface(&blockinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "usbstorage", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, USB_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, USB_BUS);

}

