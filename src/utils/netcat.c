#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    unsigned int count = 0;

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
        count = socket_send_tcp(FILE_G1, &local, &remote, &router, 1, &consoledata->data);

        break;

    default:
        count = socket_send_tcp(FILE_G1, &local, &remote, &router, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_reply(EVENT_DATA, count, &consoledata->data);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_G1, FILE_G0, "data"))
    {

        file_link(FILE_G1);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_listen_tcp(FILE_G1, &local, &remote, &router);

        while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_reply(EVENT_DATA, count, buffer);

        file_unlink(FILE_G1);

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
        file_walk2(FILE_G0, value);

    if (ascii_match(key, "local-address"))
        socket_bind_ipv4s(&local, value);

    if (ascii_match(key, "local-port"))
        socket_bind_tcps(&local, value, 42);

    if (ascii_match(key, "router-address"))
        socket_bind_ipv4s(&router, value);

}

void init(void)
{

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "50002", 42);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

