#include <fudge.h>
#include <kernel.h>
#include "audio.h"

static struct node_operands operands;

static unsigned int oninfo(struct audio_interface *interface, unsigned int source)
{

    if (interface->rate)
    {

        struct event_audioinfo audioinfo;

        audioinfo.rate = interface->rate;
        audioinfo.channels = interface->channels;
        audioinfo.bits = interface->bits;

        return kernel_place(interface->service.inode, source, EVENT_AUDIOINFO, sizeof (struct event_audioinfo), &audioinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onaudioconf(struct audio_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->setformat && count == sizeof (struct event_audioconf))
    {

        struct event_audioconf *audioconf = data;

        return (interface->setformat(audioconf->rate, audioconf->channels, audioconf->bits)) ? MESSAGE_OK : MESSAGE_FAILED;

    }

    return MESSAGE_FAILED;

}

static unsigned int ondata(struct audio_interface *interface, unsigned int source, unsigned int count, void *data)
{

    if (interface->write)
    {

        return (interface->write(data, count)) ? MESSAGE_OK : MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct audio_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->service.links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->service.links, source);

        case EVENT_INFO:
            return oninfo(interface, source);

        case EVENT_AUDIOCONF:
            return onaudioconf(interface, source, count, data);

        case EVENT_DATA:
            return ondata(interface, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void audio_registerinterface(struct audio_interface *interface)
{

    unsigned int inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_register(&interface->service, inode, "audio");

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void audio_initinterface(struct audio_interface *interface, unsigned int id, unsigned int (*setformat)(unsigned int rate, unsigned int channels, unsigned int bits), unsigned int (*write)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);
    service_init(&interface->service);

    interface->id = id;
    interface->setformat = setformat;
    interface->write = write;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place, 0);

}

