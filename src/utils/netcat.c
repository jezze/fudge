#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct option options[32];
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

    case '\f':
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
        channel_sendbuffer(EVENT_DATA, count, &consoledata->data);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    socket_bind_ipv4s(&local, option_getstring(options, "local-address"));
    socket_bind_tcps(&local, option_getstring(options, "local-port"), 42);
    socket_bind_ipv4s(&router, option_getstring(options, "router-address"));

    if (!file_walk2(FILE_L0, option_getstring(options, "ethernet")))
        channel_warning("Could not open ethernet");

    if (file_walk(FILE_L1, FILE_L0, "addr"))
        socket_resolvelocal(FILE_L1, &local);
    else
        channel_error("Could not find address");

    if (file_walk(FILE_G1, FILE_L0, "data"))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_G1);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_listen_tcp(FILE_G1, &local, &remote, &router);

        while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_sendbuffer(EVENT_DATA, count, buffer);

        file_unlink(FILE_G1);

    }

    else
    {

        channel_error("Could not find data");

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    option_set(options, key, value);

}

void init(void)
{

    socket_init(&local);
    socket_init(&router);
    option_add(options, "ethernet", "system:ethernet/if:0");
    option_add(options, "domain", "");
    option_add(options, "local-address", "10.0.5.1");
    option_add(options, "local-port", "50002");
    option_add(options, "router-address", "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

