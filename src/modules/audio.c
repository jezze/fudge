#include <fudge.h>
#include <kernel.h>
#include "audio.h"

static struct resource_operands operands;
static struct unitsrv unitsrv;
static struct service service;

static unsigned int oninfo(struct audio_interface *interface, unsigned int source)
{

    if (interface->rate)
    {

        struct message_audioinfo audioinfo;

        audioinfo.rate = interface->rate;
        audioinfo.channels = interface->channels;
        audioinfo.bits = interface->bits;

        return kernel_place(interface->resource.inode, source, MESSAGE_AUDIOINFO, sizeof (struct message_audioinfo), &audioinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int onaudioconf(struct audio_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct message_audioconf *audioconf = message_validate(MESSAGE_AUDIOCONF, data, count);

    if (interface->setformat && audioconf)
    {

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

        case MESSAGE_LINK:
            return kernel_linknode(&interface->resource, source);

        case MESSAGE_UNLINK:
            return kernel_unlinknode(&interface->resource, source);

        case MESSAGE_INFO:
            return oninfo(interface, source);

        case MESSAGE_AUDIOCONF:
            return onaudioconf(interface, source, count, data);

        case MESSAGE_DATA:
            return ondata(interface, source, count, data);

        }

    }

    return MESSAGE_FAILED;

}

void audio_registerinterface(struct audio_interface *interface)
{

    resource_register(&interface->resource);

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);

}

void audio_initinterface(struct audio_interface *interface, unsigned int (*setformat)(unsigned int rate, unsigned int channels, unsigned int bits), unsigned int (*write)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface, &operands);

    interface->setformat = setformat;
    interface->write = write;

}

void module_init(void)
{

    resource_operands_init(&operands, 0, operands_place, 0);

    unitsrv_init(&unitsrv, RESOURCE_AUDIOINTERFACE);
    service_init(&service, "audio");

}

void module_register(void)
{

    service_register(&service, pool_attachnode(&unitsrv.server.resource));

}

void module_unregister(void)
{

    service_unregister(&service);
    pool_detachnode(&unitsrv.server.resource);

}

