#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include <modules/driver.h>
#include <modules/pci/pci.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include "usb.h"

#define PAGESIZE                        0x1000
#define RINGSIZE                        (PAGESIZE / sizeof (struct trb))
#define DEVICES                         8
#define SCRATCHPADS                     16
#define INTERFACES                      3
#define DEVICEPAGES                     (6 + BULKPAGES + INTERFACES)
#define REPORTSIZE                      512
#define BULKPAGES                       4
#define PAGES                           (4 + SCRATCHPADS + DEVICES * DEVICEPAGES)
#define LIMIT_POLL                      1000000
#define CAP_LENGTH                      0x00
#define CAP_HCSPARAMS1                  0x04
#define CAP_HCSPARAMS2                  0x08
#define CAP_HCCPARAMS1                  0x10
#define CAP_HCCPARAMS1_CSZ              (1 << 2)
#define CAP_DBOFF                       0x14
#define CAP_RTSOFF                      0x18
#define OP_USBCMD                       0x00
#define OP_USBCMD_RS                    (1 << 0)
#define OP_USBCMD_HCRST                 (1 << 1)
#define OP_USBCMD_INTE                  (1 << 2)
#define OP_USBSTS                       0x04
#define OP_USBSTS_HCH                   (1 << 0)
#define OP_USBSTS_EINT                  (1 << 3)
#define OP_USBSTS_CNR                   (1 << 11)
#define OP_CRCR                         0x18
#define OP_DCBAAP                       0x30
#define OP_CONFIG                       0x38
#define OP_PORTSC                       0x400
#define PORTSC_CCS                      (1 << 0)
#define PORTSC_PED                      (1 << 1)
#define PORTSC_PR                       (1 << 4)
#define PORTSC_PRC                      (1 << 21)
#define PORTSC_CHANGES                  (0x7F << 17)
#define PORTSC_NEUTRAL                  ((1 << 0) | (1 << 3) | (0xF << 10) | (1 << 30) | (0xF << 5) | (1 << 9) | (0x3 << 14) | (0x7 << 25))
#define IR_IMAN                         0x20
#define IR_IMAN_IP                      (1 << 0)
#define IR_IMAN_IE                      (1 << 1)
#define IR_IMOD                         0x24
#define IR_ERSTSZ                       0x28
#define IR_ERSTBA                       0x30
#define IR_ERDP                         0x38
#define IR_ERDP_EHB                     (1 << 3)
#define EXTCAP_LEGACY                   1
#define EXTCAP_LEGACY_BIOS              (1 << 16)
#define EXTCAP_LEGACY_OS                (1 << 24)
#define TRB_CYCLE                       (1 << 0)
#define TRB_LINK_TC                     (1 << 1)
#define TRB_ISP                         (1 << 2)
#define TRB_ED                          (1 << 2)
#define TRB_CHAIN                       (1 << 4)
#define TRB_IOC                         (1 << 5)
#define TRB_IDT                         (1 << 6)
#define TRB_DIR_IN                      (1 << 16)
#define TRB_TYPE(t)                     ((t) << 10)
#define TRB_GETTYPE(c)                  (((c) >> 10) & 0x3F)
#define TRB_NORMAL                      1
#define TRB_SETUP                       2
#define TRB_DATA                        3
#define TRB_STATUS                      4
#define TRB_LINK                        6
#define TRB_EVENTDATA                   7
#define TRB_ENABLESLOT                  9
#define TRB_DISABLESLOT                 10
#define TRB_ADDRESSDEVICE               11
#define TRB_CONFIGUREENDPOINT           12
#define TRB_EVALUATECONTEXT             13
#define TRB_TRANSFEREVENT               32
#define TRB_COMMANDEVENT                33
#define TRB_PORTEVENT                   34
#define COMPLETION_SUCCESS              1
#define COMPLETION_SHORTPACKET          13
#define EP_CONTROL                      4
#define EP_BULKOUT                      2
#define EP_BULKIN                       6
#define EP_INTERRUPTIN                  7
#define SPEED_FULL                      1
#define SPEED_LOW                       2
#define SPEED_HIGH                      3
#define REQUEST_GETDESCRIPTOR           6
#define REQUEST_SETCONFIGURATION        9
#define DESCRIPTOR_DEVICE               1
#define DESCRIPTOR_CONFIGURATION        2
#define DESCRIPTOR_INTERFACE            4
#define DESCRIPTOR_ENDPOINT             5

struct trb
{

    unsigned int parameter0;
    unsigned int parameter1;
    unsigned int status;
    unsigned int control;

};

struct trbring
{

    volatile struct trb *trbs;
    unsigned int index;
    unsigned int cycle;

};

struct interface
{

    unsigned int number;
    unsigned int class;
    unsigned int subclass;
    unsigned int protocol;
    unsigned int endpoint;
    unsigned int endpointsize;
    unsigned int endpointinterval;
    struct trbring interrupt;
    void (*callback)(unsigned int id, void *data, unsigned int count);
    unsigned char report[64];
    unsigned int reportcount;
    unsigned int pending;

};

struct device
{

    unsigned int slot;
    unsigned int port;
    unsigned int speed;
    unsigned int maxpacket;
    unsigned int configuration;
    unsigned int bulkin;
    unsigned int bulkinsize;
    unsigned int bulkout;
    unsigned int bulkoutsize;
    unsigned char *input;
    unsigned char *output;
    unsigned char *buffer;
    unsigned char *bulkbuffer;
    struct trbring control;
    struct trbring bulkinring;
    struct trbring bulkoutring;
    struct interface interfaces[INTERFACES];
    unsigned int ninterfaces;

};

struct result
{

    unsigned int done;
    unsigned int code;
    unsigned int slot;
    unsigned int length;
    unsigned int ed;

};

static struct base_bus bus;
static struct spinlock spinlock;
static unsigned char memory[(PAGES + 1) * PAGESIZE];
static unsigned char *pages;
static unsigned long mmio;
static unsigned long op;
static unsigned long rt;
static unsigned long db;
static unsigned int contextsize;
static unsigned int ports;
static volatile unsigned int *dcbaa;
static struct trbring command;
static struct trbring event;
static struct result commandresult;
static struct result transferresult;
static struct device devices[DEVICES];
static unsigned char portchanges[256];
static unsigned int hotplugging;
static unsigned int pciid;
static unsigned int irq;
static unsigned int msi;

static unsigned int gethigh(void *address)
{

    /* two shifts, as one shift of 32 is too wide for a 32 bit long */
    return ((unsigned long)address >> 16) >> 16;

}

static unsigned int read32(unsigned long address)
{

    return *(volatile unsigned int *)address;

}

static void write32(unsigned long address, unsigned int value)
{

    *(volatile unsigned int *)address = value;

}

static void write64(unsigned long address, void *value, unsigned int flags)
{

    write32(address, (unsigned long)value | flags);
    write32(address + 4, gethigh(value));

}

static void *getpage(unsigned int index)
{

    void *page = pages + index * PAGESIZE;

    buffer_clear(page, PAGESIZE);

    return page;

}

static void initring(struct trbring *ring, void *page)
{

    ring->trbs = page;
    ring->index = 0;
    ring->cycle = 1;

}

static void enqueue(struct trbring *ring, unsigned int parameter0, unsigned int parameter1, unsigned int status, unsigned int control)
{

    volatile struct trb *trb = &ring->trbs[ring->index];

    trb->parameter0 = parameter0;
    trb->parameter1 = parameter1;
    trb->status = status;
    trb->control = control | ring->cycle;

    ring->index++;

    /* the last place in the ring links back to the start and flips the cycle bit, chained inside a td */
    if (ring->index == RINGSIZE - 1)
    {

        volatile struct trb *link = &ring->trbs[ring->index];

        link->parameter0 = (unsigned long)ring->trbs;
        link->parameter1 = gethigh((void *)ring->trbs);
        link->status = 0;
        link->control = TRB_TYPE(TRB_LINK) | TRB_LINK_TC | (control & TRB_CHAIN) | ring->cycle;

        ring->index = 0;
        ring->cycle ^= 1;

    }

}

static void ringdoorbell(unsigned int slot, unsigned int target)
{

    write32(db + slot * 4, target);

}

static struct device *finddevice(unsigned int slot)
{

    unsigned int i;

    for (i = 0; i < DEVICES; i++)
    {

        if (devices[i].slot && devices[i].slot == slot)
            return &devices[i];

    }

    return 0;

}

static unsigned char *getreportbuffer(struct device *device, unsigned int index)
{

    return device->buffer + PAGESIZE / 2 + index * REPORTSIZE;

}

static void queueinterrupt(struct device *device, unsigned int index)
{

    struct interface *interface = &device->interfaces[index];
    unsigned char *buffer = getreportbuffer(device, index);

    enqueue(&interface->interrupt, (unsigned long)buffer, gethigh(buffer), interface->endpointsize, TRB_TYPE(TRB_NORMAL) | TRB_IOC | TRB_ISP);
    ringdoorbell(device->slot, interface->endpoint);

}

static void handletransfer(volatile struct trb *trb)
{

    unsigned int slot = trb->control >> 24;
    unsigned int endpoint = (trb->control >> 16) & 0x1F;
    unsigned int code = trb->status >> 24;
    unsigned int remaining = trb->status & 0xFFFFFF;
    struct device *device = finddevice(slot);
    unsigned int i;

    for (i = 0; device && i < device->ninterfaces; i++)
    {

        struct interface *interface = &device->interfaces[i];

        if (interface->endpoint && endpoint == interface->endpoint)
        {

            if ((code == COMPLETION_SUCCESS || code == COMPLETION_SHORTPACKET) && interface->callback)
            {

                unsigned int count = interface->endpointsize - remaining;

                interface->reportcount = (count > sizeof (interface->report)) ? sizeof (interface->report) : count;
                interface->pending = 1;

                buffer_copy(interface->report, getreportbuffer(device, i), interface->reportcount);

            }

            queueinterrupt(device, i);

            return;

        }

    }

    {

        transferresult.code = code;
        transferresult.slot = slot;
        transferresult.length = remaining;
        transferresult.ed = trb->control & TRB_ED;
        transferresult.done = 1;

    }

}

static void handlecommand(volatile struct trb *trb)
{

    commandresult.code = trb->status >> 24;
    commandresult.slot = trb->control >> 24;
    commandresult.done = 1;

}

static void process(void)
{

    unsigned int handled = 0;

    while ((event.trbs[event.index].control & TRB_CYCLE) == event.cycle)
    {

        volatile struct trb *trb = &event.trbs[event.index];

        switch (TRB_GETTYPE(trb->control))
        {

        case TRB_TRANSFEREVENT:
            handletransfer(trb);

            break;

        case TRB_COMMANDEVENT:
            handlecommand(trb);

            break;

        case TRB_PORTEVENT:
            portchanges[trb->parameter0 >> 24] = 1;

            break;

        }

        event.index++;

        if (event.index == RINGSIZE)
        {

            event.index = 0;
            event.cycle ^= 1;

        }

        handled = 1;

    }

    if (handled)
        write64(rt + IR_ERDP, (void *)&event.trbs[event.index], IR_ERDP_EHB);

}

static void hotplug(void);

static void deliver(void)
{

    unsigned int i;
    unsigned int k;

    for (i = 0; i < DEVICES; i++)
    {

        for (k = 0; k < INTERFACES; k++)
        {

            struct device *device = &devices[i];
            struct interface *interface = &device->interfaces[k];
            void (*callback)(unsigned int id, void *data, unsigned int count) = 0;
            unsigned char report[64];
            unsigned int count = 0;
            unsigned int id = 0;

            spinlock_acquire(&spinlock);

            if (device->slot && k < device->ninterfaces && interface->pending && interface->callback)
            {

                callback = interface->callback;
                count = interface->reportcount;
                id = device->slot * 4 + k;

                buffer_copy(report, interface->report, count);

            }

            interface->pending = 0;

            spinlock_release(&spinlock);

            if (callback)
                callback(id, report, count);

        }

    }

}

static void handleirq(unsigned int irq)
{

    unsigned int run;

    spinlock_acquire(&spinlock);
    write32(op + OP_USBSTS, OP_USBSTS_EINT);
    write32(rt + IR_IMAN, read32(rt + IR_IMAN) | IR_IMAN_IP);
    process();

    run = !hotplugging;
    hotplugging = 1;

    spinlock_release(&spinlock);
    deliver();

    if (!run)
        return;

    hotplug();
    spinlock_acquire(&spinlock);

    hotplugging = 0;

    spinlock_release(&spinlock);

}

static unsigned int waitresult(struct result *result)
{

    unsigned int i;

    for (i = 0; i < LIMIT_POLL; i++)
    {

        process();

        if (result->done)
            return 1;

    }

    return 0;

}

static unsigned int runcommand(void *parameter, unsigned int control)
{

    commandresult.done = 0;

    enqueue(&command, (unsigned long)parameter, gethigh(parameter), 0, control);
    ringdoorbell(0, 0);

    return waitresult(&commandresult) && commandresult.code == COMPLETION_SUCCESS;

}

static unsigned int runcontrol(struct device *device, unsigned char type, unsigned char request, unsigned short value, unsigned short index, unsigned short length, void *data)
{

    unsigned int in = type & USB_REQUEST_IN;

    if (length > PAGESIZE / 2)
        return 0;

    if (data && !in)
        buffer_copy(device->buffer, data, length);

    transferresult.done = 0;

    enqueue(&device->control, type | (request << 8) | ((unsigned int)value << 16), index | ((unsigned int)length << 16), 8, TRB_TYPE(TRB_SETUP) | TRB_IDT | ((length) ? ((in) ? (3 << 16) : (2 << 16)) : 0));

    if (length)
        enqueue(&device->control, (unsigned long)device->buffer, gethigh(device->buffer), length, TRB_TYPE(TRB_DATA) | ((in) ? TRB_DIR_IN : 0));

    enqueue(&device->control, 0, 0, 0, TRB_TYPE(TRB_STATUS) | TRB_IOC | ((length && in) ? 0 : TRB_DIR_IN));
    ringdoorbell(device->slot, 1);

    if (!waitresult(&transferresult) || transferresult.code != COMPLETION_SUCCESS)
        return 0;

    if (data && in)
        buffer_copy(data, device->buffer, length);

    return 1;

}

static volatile unsigned int *getinputcontext(struct device *device, unsigned int index)
{

    return (volatile unsigned int *)(device->input + index * contextsize);

}

static unsigned int getdefaultpacket(unsigned int speed)
{

    switch (speed)
    {

    case SPEED_FULL:
    case SPEED_LOW:
        return 8;

    case SPEED_HIGH:
        return 64;

    default:
        return 512;

    }

}

static unsigned int addressdevice(struct device *device)
{

    volatile unsigned int *control = getinputcontext(device, 0);
    volatile unsigned int *slot = getinputcontext(device, 1);
    volatile unsigned int *ep0 = getinputcontext(device, 2);

    control[1] = (1 << 0) | (1 << 1);
    slot[0] = (device->speed << 20) | (1 << 27);
    slot[1] = device->port << 16;
    ep0[1] = (3 << 1) | (EP_CONTROL << 3) | (device->maxpacket << 16);
    ep0[2] = (unsigned long)device->control.trbs | 1;
    ep0[3] = gethigh((void *)device->control.trbs);
    ep0[4] = 8;

    dcbaa[device->slot * 2] = (unsigned long)device->output;
    dcbaa[device->slot * 2 + 1] = gethigh(device->output);

    return runcommand(device->input, TRB_TYPE(TRB_ADDRESSDEVICE) | (device->slot << 24));

}

static unsigned int updatemaxpacket(struct device *device, unsigned int maxpacket)
{

    volatile unsigned int *control = getinputcontext(device, 0);
    volatile unsigned int *ep0 = getinputcontext(device, 2);

    device->maxpacket = maxpacket;
    control[0] = 0;
    control[1] = (1 << 1);
    ep0[1] = (3 << 1) | (EP_CONTROL << 3) | (maxpacket << 16);

    return runcommand(device->input, TRB_TYPE(TRB_EVALUATECONTEXT) | (device->slot << 24));

}

static unsigned int getinterval(struct device *device, unsigned int interval)
{

    unsigned int exponent = 0;

    if (device->speed == SPEED_FULL || device->speed == SPEED_LOW)
    {

        /* frames of 1 ms, as a power of two of 125 us */
        unsigned int units = interval * 8;

        while ((2u << exponent) <= units)
            exponent++;

        return (exponent < 3) ? 3 : (exponent > 10) ? 10 : exponent;

    }

    return (interval) ? ((interval - 1 > 15) ? 15 : interval - 1) : 0;

}

static void parseconfiguration(struct device *device, unsigned char *data, unsigned int length)
{

    struct interface *interface = 0;
    unsigned int offset = 0;

    while (offset + 2 <= length && data[offset])
    {

        unsigned char *descriptor = data + offset;

        if (descriptor[1] == DESCRIPTOR_INTERFACE)
        {

            interface = 0;

            if (descriptor[3] == 0 && device->ninterfaces < INTERFACES)
            {

                interface = &device->interfaces[device->ninterfaces++];
                interface->number = descriptor[2];
                interface->class = descriptor[5];
                interface->subclass = descriptor[6];
                interface->protocol = descriptor[7];

            }

        }

        else if (descriptor[1] == DESCRIPTOR_ENDPOINT && interface)
        {

            unsigned int in = descriptor[2] & 0x80;
            unsigned int dci = (descriptor[2] & 0x0F) * 2 + ((in) ? 1 : 0);
            unsigned int size = (descriptor[4] | (descriptor[5] << 8)) & 0x7FF;

            switch (descriptor[3] & 0x03)
            {

            case 2:
                if (in && !device->bulkin)
                {

                    device->bulkin = dci;
                    device->bulkinsize = size;

                }

                if (!in && !device->bulkout)
                {

                    device->bulkout = dci;
                    device->bulkoutsize = size;

                }

                break;

            case 3:
                if (in && !interface->endpoint)
                {

                    interface->endpoint = dci;
                    interface->endpointsize = (size > REPORTSIZE) ? REPORTSIZE : size;
                    interface->endpointinterval = getinterval(device, descriptor[6]);

                }

                break;

            }

        }

        offset += descriptor[0];

    }

}

static void setendpoint(struct device *device, unsigned int dci, unsigned int type, unsigned int size, unsigned int interval, struct trbring *ring)
{

    volatile unsigned int *endpoint = getinputcontext(device, dci + 1);

    endpoint[0] = interval << 16;
    endpoint[1] = (3 << 1) | (type << 3) | (size << 16);
    endpoint[2] = (unsigned long)ring->trbs | 1;
    endpoint[3] = gethigh((void *)ring->trbs);
    endpoint[4] = (type == EP_INTERRUPTIN) ? size | (size << 16) : size;

}

static unsigned int configureendpoints(struct device *device)
{

    volatile unsigned int *control = getinputcontext(device, 0);
    volatile unsigned int *slot = getinputcontext(device, 1);
    unsigned int last = 1;
    unsigned int i;

    control[0] = 0;
    control[1] = (1 << 0);

    for (i = 0; i < device->ninterfaces; i++)
    {

        struct interface *interface = &device->interfaces[i];

        if (!interface->endpoint)
            continue;

        setendpoint(device, interface->endpoint, EP_INTERRUPTIN, interface->endpointsize, interface->endpointinterval, &interface->interrupt);

        control[1] |= 1 << interface->endpoint;
        last = (interface->endpoint > last) ? interface->endpoint : last;

    }

    if (device->bulkin)
    {

        setendpoint(device, device->bulkin, EP_BULKIN, device->bulkinsize, 0, &device->bulkinring);

        control[1] |= 1 << device->bulkin;
        last = (device->bulkin > last) ? device->bulkin : last;

    }

    if (device->bulkout)
    {

        setendpoint(device, device->bulkout, EP_BULKOUT, device->bulkoutsize, 0, &device->bulkoutring);

        control[1] |= 1 << device->bulkout;
        last = (device->bulkout > last) ? device->bulkout : last;

    }

    if (last == 1)
        return 1;

    slot[0] = (slot[0] & ~(0x1F << 27)) | (last << 27);

    return runcommand(device->input, TRB_TYPE(TRB_CONFIGUREENDPOINT) | (device->slot << 24));

}

static unsigned int setupdevice(struct device *device)
{

    unsigned char descriptor[18];
    unsigned char configuration[256];
    unsigned int length;

    if (!addressdevice(device))
        return 0;

    if (!runcontrol(device, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, DESCRIPTOR_DEVICE << 8, 0, 8, descriptor))
        return 0;

    if (descriptor[7] && descriptor[7] != device->maxpacket && device->speed < 4)
    {

        if (!updatemaxpacket(device, descriptor[7]))
            return 0;

    }

    if (!runcontrol(device, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, DESCRIPTOR_CONFIGURATION << 8, 0, 9, configuration))
        return 0;

    length = configuration[2] | (configuration[3] << 8);

    if (length > sizeof (configuration))
        length = sizeof (configuration);

    if (!runcontrol(device, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, DESCRIPTOR_CONFIGURATION << 8, 0, length, configuration))
        return 0;

    parseconfiguration(device, configuration, length);

    if (!runcontrol(device, USB_REQUEST_OUT, REQUEST_SETCONFIGURATION, configuration[5], 0, 0, 0))
        return 0;

    return configureendpoints(device);

}

static unsigned int resetport(unsigned int port)
{

    unsigned long portsc = op + OP_PORTSC + (port - 1) * 0x10;
    unsigned int i;

    if (!(read32(portsc) & PORTSC_CCS))
        return 0;

    /* usb 3 ports are enabled already, usb 2 ports need a reset to be */
    if (!(read32(portsc) & PORTSC_PED))
    {

        write32(portsc, (read32(portsc) & PORTSC_NEUTRAL) | PORTSC_PR);

        for (i = 0; i < LIMIT_POLL && !(read32(portsc) & PORTSC_PRC); i++);

    }

    write32(portsc, (read32(portsc) & PORTSC_NEUTRAL) | PORTSC_CHANGES);

    return (read32(portsc) & PORTSC_PED) ? (read32(portsc) >> 10) & 0x0F : 0;

}

static void disableslot(unsigned int slot)
{

    runcommand(0, TRB_TYPE(TRB_DISABLESLOT) | (slot << 24));

    dcbaa[slot * 2] = 0;
    dcbaa[slot * 2 + 1] = 0;

}

static struct device *findport(unsigned int port)
{

    unsigned int i;

    for (i = 0; i < DEVICES; i++)
    {

        if (devices[i].slot && devices[i].port == port)
            return &devices[i];

    }

    return 0;

}

static struct device *enumerateport(unsigned int port)
{

    unsigned int speed = resetport(port);
    unsigned int index;
    unsigned int i;

    if (!speed)
        return 0;

    for (index = 0; index < DEVICES; index++)
    {

        struct device *device = &devices[index];
        unsigned int page = 4 + SCRATCHPADS + index * DEVICEPAGES;

        if (device->slot)
            continue;

        if (!runcommand(0, TRB_TYPE(TRB_ENABLESLOT)))
            return 0;

        buffer_clear(device, sizeof (struct device));

        device->port = port;
        device->speed = speed;
        device->maxpacket = getdefaultpacket(speed);
        device->input = getpage(page + 0);
        device->output = getpage(page + 1);
        device->buffer = getpage(page + 3);
        device->bulkbuffer = getpage(page + 6);

        for (i = 1; i < BULKPAGES; i++)
            getpage(page + 6 + i);

        initring(&device->control, getpage(page + 2));
        initring(&device->bulkinring, getpage(page + 4));
        initring(&device->bulkoutring, getpage(page + 5));

        for (i = 0; i < INTERFACES; i++)
            initring(&device->interfaces[i].interrupt, getpage(page + 6 + BULKPAGES + i));

        device->slot = commandresult.slot;

        if (setupdevice(device))
            return device;

        disableslot(device->slot);

        device->slot = 0;

        return 0;

    }

    return 0;

}

static void enumerate(void)
{

    unsigned int port;

    for (port = 1; port <= ports; port++)
        enumerateport(port);

}

static void hotplug(void)
{

    unsigned int port;

    for (port = 1; port <= ports && port < 256; port++)
    {

        unsigned long portsc = op + OP_PORTSC + (port - 1) * 0x10;
        struct device *device;
        unsigned int slot = 0;
        unsigned int i;

        spinlock_acquire(&spinlock);

        if (portchanges[port])
        {

            portchanges[port] = 0;
            device = findport(port);

            if (device && !(read32(portsc) & PORTSC_CCS))
            {

                slot = device->slot;

                for (i = 0; i < device->ninterfaces; i++)
                    device->interfaces[i].callback = 0;

            }

            else if (!device && (read32(portsc) & PORTSC_CCS))
            {

                device = enumerateport(port);

                if (device)
                    slot = device->slot;

                device = 0;

            }

            else
            {

                device = 0;

            }

            write32(portsc, (read32(portsc) & PORTSC_NEUTRAL) | PORTSC_CHANGES);

        }

        else
        {

            device = 0;

        }

        spinlock_release(&spinlock);

        if (!slot)
            continue;

        if (device)
        {

            for (i = 0; i < device->ninterfaces; i++)
                base_detachdevice(USB_BUS, slot * 4 + i);

            spinlock_acquire(&spinlock);
            disableslot(slot);

            device->slot = 0;

            spinlock_release(&spinlock);

        }

        else
        {

            device = finddevice(slot);

            for (i = 0; device && i < device->ninterfaces; i++)
                base_attachdevice(USB_BUS, slot * 4 + i);

        }

    }

}

static void takeownership(void)
{

    unsigned int offset = (read32(mmio + CAP_HCCPARAMS1) >> 16) << 2;
    unsigned int i;

    while (offset)
    {

        unsigned long capability = mmio + offset;
        unsigned int value = read32(capability);

        if ((value & 0xFF) == EXTCAP_LEGACY)
        {

            write32(capability, value | EXTCAP_LEGACY_OS);

            for (i = 0; i < LIMIT_POLL && (read32(capability) & EXTCAP_LEGACY_BIOS); i++);

            return;

        }

        offset = ((value >> 8) & 0xFF) ? offset + (((value >> 8) & 0xFF) << 2) : 0;

    }

}

static unsigned int start(void)
{

    unsigned int hcsparams1 = read32(mmio + CAP_HCSPARAMS1);
    unsigned int hcsparams2 = read32(mmio + CAP_HCSPARAMS2);
    unsigned int scratchpads = (((hcsparams2 >> 21) & 0x1F) << 5) | ((hcsparams2 >> 27) & 0x1F);
    unsigned int slots = hcsparams1 & 0xFF;
    volatile unsigned int *erst;
    unsigned int i;

    op = mmio + (read32(mmio + CAP_LENGTH) & 0xFF);
    rt = mmio + (read32(mmio + CAP_RTSOFF) & ~0x1F);
    db = mmio + (read32(mmio + CAP_DBOFF) & ~0x3);
    ports = hcsparams1 >> 24;
    contextsize = (read32(mmio + CAP_HCCPARAMS1) & CAP_HCCPARAMS1_CSZ) ? 64 : 32;

    if (scratchpads > SCRATCHPADS)
        return 0;

    takeownership();

    write32(op + OP_USBCMD, read32(op + OP_USBCMD) & ~OP_USBCMD_RS);

    for (i = 0; i < LIMIT_POLL && !(read32(op + OP_USBSTS) & OP_USBSTS_HCH); i++);

    write32(op + OP_USBCMD, OP_USBCMD_HCRST);

    for (i = 0; i < LIMIT_POLL && (read32(op + OP_USBCMD) & OP_USBCMD_HCRST); i++);
    for (i = 0; i < LIMIT_POLL && (read32(op + OP_USBSTS) & OP_USBSTS_CNR); i++);

    if (i == LIMIT_POLL)
        return 0;

    if (slots > DEVICES)
        slots = DEVICES;

    dcbaa = getpage(0);

    initring(&command, getpage(1));
    initring(&event, getpage(2));

    erst = getpage(3);
    erst[0] = (unsigned long)event.trbs;
    erst[1] = gethigh((void *)event.trbs);
    erst[2] = RINGSIZE;

    if (scratchpads)
    {

        volatile unsigned int *array = (volatile unsigned int *)((unsigned char *)erst + PAGESIZE / 2);

        for (i = 0; i < scratchpads; i++)
        {

            void *page = getpage(4 + i);

            array[i * 2] = (unsigned long)page;
            array[i * 2 + 1] = gethigh(page);

        }

        dcbaa[0] = (unsigned long)array;
        dcbaa[1] = gethigh((void *)array);

    }

    write32(op + OP_CONFIG, slots);
    write64(op + OP_DCBAAP, (void *)dcbaa, 0);
    write64(op + OP_CRCR, (void *)command.trbs, 1);
    write32(rt + IR_ERSTSZ, 1);
    write64(rt + IR_ERDP, (void *)event.trbs, 0);
    write64(rt + IR_ERSTBA, (void *)erst, 0);
    write32(rt + IR_IMOD, 0);
    write32(op + OP_USBCMD, OP_USBCMD_RS);

    for (i = 0; i < LIMIT_POLL && (read32(op + OP_USBSTS) & OP_USBSTS_HCH); i++);

    return i < LIMIT_POLL;

}

static void enableinterrupts(void)
{

    msi = apic_setmsiroutine(handleirq);

    if (msi && !pci_setmsix(pciid, 0, apic_getmsiaddress(), apic_getmsidata(msi)) && !pci_setmsi(pciid, apic_getmsiaddress(), apic_getmsidata(msi)))
    {

        apic_unsetroutine(msi, handleirq);

        msi = 0;

    }

    if (!msi)
    {

        irq = pci_getirq(pciid);

        pic_setroutine(irq, handleirq);
        apic_setroutine(irq, handleirq);

    }

    write32(rt + IR_IMAN, IR_IMAN_IP | IR_IMAN_IE);
    write32(op + OP_USBCMD, read32(op + OP_USBCMD) | OP_USBCMD_INTE);

}

static unsigned int findcontroller(void)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_BUS)))
    {

        struct base_bus *pcibus = current->data;

        if (pcibus->type == PCI_BUS)
        {

            unsigned int id = 0;

            while ((id = pcibus->next(id)))
            {

                if (pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_SERIAL && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_SERIAL_USB && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x30)
                    return id;

            }

        }

    }

    return 0;

}

static struct device *getdevice(unsigned int id)
{

    return finddevice(id / 4);

}

static struct interface *getinterface(unsigned int id)
{

    struct device *device = getdevice(id);

    return (device && id % 4 < device->ninterfaces) ? &device->interfaces[id % 4] : 0;

}

unsigned int usb_getclass(unsigned int id)
{

    struct interface *interface = getinterface(id);

    return (interface) ? interface->class : 0;

}

unsigned int usb_getsubclass(unsigned int id)
{

    struct interface *interface = getinterface(id);

    return (interface) ? interface->subclass : 0;

}

unsigned int usb_getprotocol(unsigned int id)
{

    struct interface *interface = getinterface(id);

    return (interface) ? interface->protocol : 0;

}

unsigned int usb_getinterface(unsigned int id)
{

    struct interface *interface = getinterface(id);

    return (interface) ? interface->number : 0;

}

unsigned int usb_control(unsigned int id, unsigned char type, unsigned char request, unsigned short value, unsigned short index, unsigned short length, void *data)
{

    struct device *device = getdevice(id);
    unsigned int status;

    if (!device)
        return 0;

    spinlock_acquire(&spinlock);

    status = runcontrol(device, type, request, value, index, length, data);

    spinlock_release(&spinlock);

    return status;

}

unsigned int usb_listen(unsigned int id, void (*callback)(unsigned int id, void *data, unsigned int count))
{

    struct device *device = getdevice(id);
    struct interface *interface = getinterface(id);

    if (!interface || !interface->endpoint)
        return 0;

    spinlock_acquire(&spinlock);

    interface->callback = callback;

    queueinterrupt(device, id % 4);
    spinlock_release(&spinlock);

    return 1;

}

unsigned int usb_bulk(unsigned int id, unsigned int in, void *data, unsigned int count)
{

    struct device *device = getdevice(id);
    struct trbring *ring;
    unsigned int dci;
    unsigned int offset;
    unsigned int status = 0;

    if (!device || count > BULKPAGES * PAGESIZE)
        return 0;

    dci = (in) ? device->bulkin : device->bulkout;
    ring = (in) ? &device->bulkinring : &device->bulkoutring;

    if (!dci)
        return 0;

    spinlock_acquire(&spinlock);

    if (!in)
        buffer_copy(device->bulkbuffer, data, count);

    transferresult.done = 0;

    /* one trb per page, so none crosses a 64 KB boundary */
    for (offset = 0; offset < count || !count; offset += PAGESIZE)
    {

        unsigned int length = (count - offset > PAGESIZE) ? PAGESIZE : count - offset;
        unsigned char *buffer = device->bulkbuffer + offset;

        enqueue(ring, (unsigned long)buffer, gethigh(buffer), length, TRB_TYPE(TRB_NORMAL) | TRB_CHAIN);

        if (offset + length >= count)
            break;

    }

    /* a short packet skips to this trb, whose event holds the bytes moved by the whole td */
    enqueue(ring, 0, 0, 0, TRB_TYPE(TRB_EVENTDATA) | TRB_IOC);

    ringdoorbell(device->slot, dci);

    if (waitresult(&transferresult) && transferresult.ed && (transferresult.code == COMPLETION_SUCCESS || transferresult.code == COMPLETION_SHORTPACKET))
    {

        status = (transferresult.length > count) ? count : transferresult.length;

        if (in)
            buffer_copy(data, device->bulkbuffer, status);

    }

    spinlock_release(&spinlock);

    return status;

}

void usb_unlisten(unsigned int id)
{

    struct interface *interface = getinterface(id);

    if (interface)
        interface->callback = 0;

}

static void bus_setup(void)
{

    pciid = findcontroller();

    if (!pciid)
        return;

    /* a 32 bit kernel can't reach registers the firmware put above 4 GB, which ovmf does with 64 bit bars */
    if (sizeof (unsigned long) == 4 && (pci_ind(pciid, PCI_CONFIG_BAR0) & 0x06) == 0x04 && pci_ind(pciid, PCI_CONFIG_BAR1))
        return;

    mmio = pci_getbar(pciid, 0);

    if (!mmio)
        return;

    pages = (unsigned char *)(((unsigned long)memory + PAGESIZE - 1) & ~(PAGESIZE - 1UL));

    arch_kmap(mmio, mmio, 0x10000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    pci_setmaster(pciid);

    if (!start())
        return;

    enumerate();
    enableinterrupts();

}

static unsigned int bus_next(unsigned int id)
{

    unsigned int next = 0;
    unsigned int i;
    unsigned int k;

    for (i = 0; i < DEVICES; i++)
    {

        for (k = 0; devices[i].slot && k < devices[i].ninterfaces; k++)
        {

            unsigned int current = devices[i].slot * 4 + k;

            if (current > id && (!next || current < next))
                next = current;

        }

    }

    return next;

}

void module_init(void)
{

    base_initbus(&bus, USB_BUS, "usb", bus_setup, bus_next);

}

void module_register(void)
{

    base_registerbus(&bus);

}

void module_unregister(void)
{

    base_unregisterbus(&bus);

}

