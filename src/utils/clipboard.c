#include <fudge.h>
#include <abi.h>

#define CLIPBOARD_SIZE                  0x8000

static char data[CLIPBOARD_SIZE];
static unsigned int count;
static unsigned int started;

static void writepiece(void *buffer, unsigned int length)
{

    unsigned int service = channel_lookup(option_getstring("clipboard-service"));

    if (!service)
        return;

    if (!started)
        channel_send(0, service, EVENT_CLIPBOARDWRITE, 0, 0);

    if (length)
        channel_send(0, service, EVENT_CLIPBOARDWRITE, length, buffer);

    started = 1;

}

static void onclipboardread(struct message *message)
{

    unsigned int source = message->source;
    unsigned int offset;

    for (offset = 0; offset < count; offset += MESSAGE_SIZE)
        channel_send(0, source, EVENT_CLIPBOARDDATA, (count - offset < MESSAGE_SIZE) ? count - offset : MESSAGE_SIZE, data + offset);

    channel_send(0, source, EVENT_CLIPBOARDDATA, 0, 0);

}

static void onclipboardwrite(struct message *message)
{

    if (message->length)
        count += buffer_write(data, CLIPBOARD_SIZE, message->data, message->length, count);
    else
        count = 0;

}

static void ondata(struct message *message)
{

    if (option_getdecimal("write"))
        writepiece(message->data, message->length);

}

static void onmain(struct message *message)
{

    char *name = option_getstring("clipboard-service");
    unsigned int service;

    if (option_getdecimal("service"))
    {

        call_announce(0, cstring_length(name), name);
        channel_hold(0);

        return;

    }

    service = channel_lookup(name);

    if (!service)
    {

        channel_send_fmt(0, message->source, EVENT_ERROR, "Service not found: %s\n", name);

        return;

    }

    if (option_getdecimal("write"))
    {

        writepiece(0, 0);

    }

    else
    {

        struct message reply;

        channel_send(1, service, EVENT_CLIPBOARDREAD, 0, 0);

        while (channel_poll(1, service, EVENT_CLIPBOARDDATA, &reply) && reply.length)
            channel_send(0, message->source, EVENT_DATA, reply.length, reply.data);

    }

}

void init(void)
{

    option_add("clipboard-service", "clipboard");
    option_add("service", "0");
    option_add("write", "0");
    channel_bind(EVENT_CLIPBOARDREAD, onclipboardread);
    channel_bind(EVENT_CLIPBOARDWRITE, onclipboardwrite);
    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

