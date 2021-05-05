#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;

static void interpret(void *buffer, unsigned int count)
{

    char *data = buffer;

    if (data[0] == '/')
    {

        socket_send_tcp(FILE_G1, &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char *text = "PRIVMSG #fudge :";
        char outputdata[BUFFER_SIZE];
        unsigned int offset = 0;

        offset += buffer_write(outputdata, BUFFER_SIZE, text, ascii_length(text), offset);
        offset += buffer_write(outputdata, BUFFER_SIZE, buffer, count, offset);

        socket_send_tcp(FILE_G1, &local, &remote, &router, offset, outputdata);

    }

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (!remote.resolved)
        return;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        break;

    case '\b':
    case 0x7F:
        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        ring_write(&input, &consoledata->data, 1);
        channel_reply(channel, EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_reply(channel, EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *request = "NICK jfu_fudge\nUSER jfu_fudge 0 * :Jens Fudge\nJOIN #fudge\n";
    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_G1, FILE_G0, "data"))
    {

        file_link(FILE_G1);
        socket_resolveremote(channel, FILE_G1, &local, &router);
        socket_connect_tcp(channel, FILE_G1, &local, &remote, &router);
        socket_send_tcp(FILE_G1, &local, &remote, &router, ascii_length(request), request);

        while ((count = socket_receive_tcp(channel, FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_reply(channel, EVENT_DATA, count, buffer);

        socket_disconnect_tcp(channel, FILE_G1, &local, &remote, &router);
        file_unlink(FILE_G1);

    }

    channel_close(channel);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
    {

        file_walk2(FILE_G0, value);

    }

}

void init(struct channel *channel)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "50003", 42);
    socket_init(&remote);
    socket_bind_ipv4s(&remote, "185.30.166.37");
    socket_bind_tcps(&remote, "6667", 0);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

