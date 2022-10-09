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

    char buffer[BUFFER_SIZE];
    unsigned int count;
    struct ctrl_clocksettings settings;
    struct mtwist_state state;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        channel_error("Could not find clock device");

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        channel_error("Could not find clock device ctrl");

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings));
    mtwist_seed1(&state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_L1, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    if (!file_walk(FILE_L2, FILE_L0, "addr"))
        channel_error("Could not find ethernet device addr");

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcpv(&local, mtwist_rand(&state), mtwist_rand(&state));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L2, &local);
    file_link(FILE_G1);
    socket_resolveremote(FILE_G1, &local, &router);
    socket_listen_tcp(FILE_G1, &local, &remote, &router);

    while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
        channel_sendbuffer(EVENT_DATA, count, buffer);

    file_unlink(FILE_G1);
    channel_close();

}

void init(void)
{

    socket_init(&local);
    socket_init(&router);
    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("domain", "");
    option_add("local-address", "10.0.5.1");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

