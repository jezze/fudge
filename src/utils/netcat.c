#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket router;
static struct socket local;
static struct socket remotes[64];

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    unsigned int count = 0;

    if (!remotes[0].resolved)
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
        count = socket_send_tcp(0, option_getdecimal("ethernet-service"), &local, &remotes[0], &router, 1, &consoledata->data);

        break;

    default:
        count = socket_send_tcp(0, option_getdecimal("ethernet-service"), &local, &remotes[0], &router, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_send_buffer(0, 0 /* TODO: Should not be 0 */, EVENT_DATA, count, &consoledata->data);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[MESSAGE_SIZE];
    unsigned int count;
    struct event_clockinfo clockinfo;
    struct mtwist_state state;

    option_setdecimal("clock-service", lookup(option_getstring("clock-service")));
    option_setdecimal("ethernet-service", lookup(option_getstring("ethernet-service")));
    channel_send(0, option_getdecimal("clock-service"), EVENT_INFO);
    channel_wait_buffer(0, option_getdecimal("clock-service"), EVENT_CLOCKINFO, sizeof (struct event_clockinfo), &clockinfo);
    mtwist_seed1(&state, time_unixtime(clockinfo.year, clockinfo.month, clockinfo.day, clockinfo.hours, clockinfo.minutes, clockinfo.seconds));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcps(&local, option_getstring("local-port"), mtwist_rand(&state), mtwist_rand(&state));
    socket_resolvelocal(0, option_getdecimal("ethernet-service"), &local);
    channel_send(0, option_getdecimal("ethernet-service"), EVENT_LINK);
    socket_resolveremote(0, option_getdecimal("ethernet-service"), &local, &router);
    socket_listen_tcp(option_getdecimal("ethernet-service"), &local, remotes, 64, &router);

    while ((count = socket_receive(0, option_getdecimal("ethernet-service"), &local, remotes, 64, &router, buffer, MESSAGE_SIZE)))
        channel_send_buffer(0, source, EVENT_DATA, count, buffer);

    channel_send(0, option_getdecimal("ethernet-service"), EVENT_UNLINK);

}

void init(void)
{

    unsigned int i;

    socket_init(&local);
    socket_init(&router);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock-service", "clock");
    option_add("ethernet-service", "ethernet");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

