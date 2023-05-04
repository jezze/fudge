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
        count = socket_send_tcp(FILE_G0, &local, &remotes[0], &router, 1, &consoledata->data);

        break;

    default:
        count = socket_send_tcp(FILE_G0, &local, &remotes[0], &router, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, count, &consoledata->data);

}

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!file_walk2(FILE_L0, option_getstring("clock")))
        PANIC();

    if (!file_walk(FILE_L1, FILE_L0, "ctrl"))
        PANIC();

    file_readall(FILE_L1, &settings, sizeof (struct ctrl_clocksettings), 0);
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void setupnetwork(struct mtwist_state *state)
{

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        PANIC();

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        PANIC();

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        PANIC();

    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_tcps(&local, option_getstring("local-port"), mtwist_rand(state), mtwist_rand(state));
    socket_resolvelocal(FILE_L1, &local);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char buffer[BUFFER_SIZE];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);
    file_link(FILE_G0, 8000);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_listen_tcp(FILE_G0, &local, remotes, 64, &router);

    while ((count = socket_receive(FILE_G0, &local, remotes, 64, &router, buffer, BUFFER_SIZE)))
        channel_sendbuffer(CHANNEL_DEFAULT, EVENT_DATA, count, buffer);

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    unsigned int i;

    socket_init(&local);
    socket_init(&router);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock", "system:clock/if:0");
    option_add("ethernet", "system:ethernet/if:0");
    option_add("local-address", "10.0.5.1");
    option_add("local-port", "");
    option_add("router-address", "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);

}

