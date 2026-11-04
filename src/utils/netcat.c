#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket router;
static struct socket local;
static struct socket remotes[64];
static unsigned int ethernet;
static unsigned int output;

static void onconsoledata(struct message *message)
{

    unsigned char data[MESSAGE_SIZE];
    unsigned int count = buffer_read(data, MESSAGE_SIZE, message->data, message->length, 0);
    unsigned int i;

    if (!ethernet || !remotes[0].resolved)
        return;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\0':
        case '\f':
        case '\t':
        case '\b':
        case 0x7F:
            break;

        case '\r':
            data[i] = '\n';

        default:
            if (socket_send_tcp(0, ethernet, &local, &remotes[0], &router, 1, &data[i]))
                channel_send(0, output, MESSAGE_TYPE_DATA, 1, &data[i]);

            break;

        }

    }

}

static void onmain(struct message *message)
{

    ethernet = channel_lookup(option_getstring("ethernet-service"));
    output = message->source;

    if (ethernet)
    {

        char address[32];
        char buffer[MESSAGE_SIZE];
        unsigned int count;
        struct mtwist_state state;

        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));
        socket_bind_ipv4s(&router, system_address("router-address", "route", address, 32));
        socket_bind_ipv4s(&local, system_address("local-address", "address", address, 32));
        socket_bind_tcps(&local, option_getstring("local-port"), mtwist_rand(&state), mtwist_rand(&state));
        socket_resolvelocal(0, ethernet, &local);
        channel_send(0, ethernet, MESSAGE_TYPE_LINK, 0, 0);
        socket_resolveremote(0, ethernet, &local, &router);
        socket_listen_tcp(ethernet, &local, remotes, 64, &router);

        while ((count = socket_receive(0, ethernet, &local, remotes, 64, &router, buffer, MESSAGE_SIZE)))
            channel_send(0, output, MESSAGE_TYPE_DATA, count, buffer);

        channel_send(0, ethernet, MESSAGE_TYPE_UNLINK, 0, 0);

    }

}

void init(void)
{

    unsigned int i;

    socket_init(&local);
    socket_init(&router);

    for (i = 0; i < 64; i++)
        socket_init(&remotes[i]);

    option_add("clock-service", "clock:0");
    option_add("ethernet-service", "ethernet:0");
    option_add("local-address", "");
    option_add("local-port", "");
    option_add("router-address", "");
    channel_bind(MESSAGE_TYPE_CONSOLEDATA, onconsoledata);
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

