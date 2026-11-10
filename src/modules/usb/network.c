#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/ethernet.h>
#include "usb.h"

#define CLASS_COMMUNICATION             0x02
#define SUBCLASS_ETHERNET               0x06
#define REQUEST_GETDESCRIPTOR           0x06
#define REQUEST_GETCONFIGURATION        0x08
#define REQUEST_SETINTERFACE            0x0B
#define REQUEST_SETPACKETFILTER         0x43
#define DESCRIPTOR_DEVICE               0x01
#define DESCRIPTOR_CONFIGURATION        0x02
#define DESCRIPTOR_STRING               0x03
#define DESCRIPTOR_INTERFACE            0x04
#define DESCRIPTOR_CLASSINTERFACE       0x24
#define FUNCTIONAL_UNION                0x06
#define FUNCTIONAL_ETHERNET             0x0F
#define FILTER_ALLMULTICAST             0x02
#define FILTER_DIRECTED                 0x04
#define FILTER_BROADCAST                0x08
#define CONFIGURATIONS                  4
#define CONFIGURATIONSIZE               256
#define DEVICESIZE                      18
#define STRINGSIZE                      26
#define LANGUAGESIZE                    4
#define MACSIZE                         6
#define FRAMESIZE                       1514
#define PACKETMULTIPLE                  64

struct functions
{

    unsigned int data;
    unsigned int macstring;

};

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static unsigned char txbuffer[FRAMESIZE + 1];
static unsigned char mac[MACSIZE];
static unsigned int ready;

static unsigned int readconfiguration(unsigned int id, unsigned int index, unsigned char *configuration)
{

    unsigned int length;

    if (!usb_control(id, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, (DESCRIPTOR_CONFIGURATION << 8) | index, 0, 9, configuration))
        return 0;

    length = configuration[2] | (configuration[3] << 8);

    if (length > CONFIGURATIONSIZE)
        length = CONFIGURATIONSIZE;

    return usb_control(id, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, (DESCRIPTOR_CONFIGURATION << 8) | index, 0, length, configuration) ? length : 0;

}

static struct functions parsefunctions(unsigned int interface, unsigned char *configuration, unsigned int length)
{

    struct functions functions;
    unsigned int inside = 0;
    unsigned int offset = 0;

    functions.data = 0;
    functions.macstring = 0;

    while (offset + 2 <= length && configuration[offset])
    {

        unsigned char *descriptor = configuration + offset;

        if (descriptor[1] == DESCRIPTOR_INTERFACE)
            inside = descriptor[2] == interface && descriptor[3] == 0;

        else if (inside && descriptor[1] == DESCRIPTOR_CLASSINTERFACE && descriptor[0] >= 5 && descriptor[2] == FUNCTIONAL_UNION)
            functions.data = descriptor[4];

        else if (inside && descriptor[1] == DESCRIPTOR_CLASSINTERFACE && descriptor[0] >= 4 && descriptor[2] == FUNCTIONAL_ETHERNET)
            functions.macstring = descriptor[3];

        offset += descriptor[0];

    }

    return functions;

}

static struct functions findfunctions(unsigned int id)
{

    struct functions functions;
    unsigned char descriptor[DEVICESIZE];
    unsigned char configuration[CONFIGURATIONSIZE];
    unsigned char value[1];
    unsigned int count;
    unsigned int i;

    functions.data = 0;
    functions.macstring = 0;

    if (!usb_control(id, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, DESCRIPTOR_DEVICE << 8, 0, DEVICESIZE, descriptor))
        return functions;

    if (!usb_control(id, USB_REQUEST_IN, REQUEST_GETCONFIGURATION, 0, 0, 1, value))
        return functions;

    count = (descriptor[17] > CONFIGURATIONS) ? CONFIGURATIONS : descriptor[17];

    for (i = 0; i < count; i++)
    {

        unsigned int length = readconfiguration(id, i, configuration);

        if (length && configuration[5] == value[0])
            return parsefunctions(usb_getinterface(id), configuration, length);

    }

    return functions;

}

static unsigned int gethex(unsigned char c)
{

    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return 16;

}

static unsigned int readmac(unsigned int id, unsigned int index)
{

    unsigned char languages[LANGUAGESIZE];
    unsigned char string[STRINGSIZE];
    unsigned int language;
    unsigned int i;

    if (!index || !usb_control(id, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, DESCRIPTOR_STRING << 8, 0, LANGUAGESIZE, languages) || languages[0] < LANGUAGESIZE)
        return 0;

    language = languages[2] | (languages[3] << 8);

    if (!usb_control(id, USB_REQUEST_IN, REQUEST_GETDESCRIPTOR, (DESCRIPTOR_STRING << 8) | index, language, STRINGSIZE, string) || string[0] < STRINGSIZE)
        return 0;

    /* twelve hex digits in utf-16 */
    for (i = 0; i < MACSIZE; i++)
    {

        unsigned int high = gethex(string[2 + i * 4]);
        unsigned int low = gethex(string[4 + i * 4]);

        if (high > 15 || low > 15)
            return 0;

        mac[i] = (high << 4) | low;

    }

    return 1;

}

static void onreceive(unsigned int id, void *data, unsigned int count)
{

    ethernet_notifydata(&ethernetinterface, data, count);

}

static void ethernetinterface_getmac(unsigned char address[6])
{

    buffer_copy(address, mac, MACSIZE);

}

static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    unsigned int length = count;

    if (!ready || count > FRAMESIZE)
        return 0;

    buffer_copy(txbuffer, buffer, count);

    /* a frame filling whole packets would need a zero length packet to end it, a padding byte does the same */
    if (length % PACKETMULTIPLE == 0)
        txbuffer[length++] = 0;

    return (usb_bulk(driver.id, 0, txbuffer, length) == length) ? count : 0;

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, ethernetinterface_getmac, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == USB_BUS && usb_getclass(id) == CLASS_COMMUNICATION && usb_getsubclass(id) == SUBCLASS_ETHERNET;

}

static void driver_reset(unsigned int id)
{

    struct functions functions = findfunctions(id);

    ready = 0;

    if (!functions.data || !readmac(id, functions.macstring))
        return;

    /* the data interface's setting 0 has no endpoints, setting 1 holds the bulk pair */
    if (!usb_control(id, USB_REQUEST_OUT | USB_REQUEST_STANDARD | USB_REQUEST_INTERFACE, REQUEST_SETINTERFACE, 1, functions.data, 0, 0))
        return;

    /* optional, some adapters refuse it */
    usb_control(id, USB_REQUEST_OUT | USB_REQUEST_CLASS | USB_REQUEST_INTERFACE, REQUEST_SETPACKETFILTER, FILTER_ALLMULTICAST | FILTER_DIRECTED | FILTER_BROADCAST, usb_getinterface(id), 0, 0);

    ready = 1;

}

static void driver_attach(unsigned int id)
{

    if (!ready)
        return;

    if (!usb_listenbulk(id, onreceive))
    {

        ready = 0;

        return;

    }

    ethernet_registerinterface(&ethernetinterface);

}

static void driver_detach(unsigned int id)
{

    if (!ready)
        return;

    ethernet_unregisterinterface(&ethernetinterface);
    usb_unlistenbulk(id);

    ready = 0;

}

void module_init(void)
{

    base_initdriver(&driver, "usbnetwork", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

