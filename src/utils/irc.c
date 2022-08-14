#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;
static char domain[512];

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

        offset += cstring_write(outputdata, BUFFER_SIZE, text, offset);
        offset += buffer_write(outputdata, BUFFER_SIZE, buffer, count, offset);

        socket_send_tcp(FILE_G1, &local, &remote, &router, offset, outputdata);

    }

}

static void resolve(char *domain)
{

    unsigned int id = file_spawn("/bin/dns");

    if (id)
    {

        struct message message;
        unsigned int c;

        message_init(&message, EVENT_OPTION);
        message_putstringz(&message, "domain");
        message_putstringz(&message, domain);
        channel_redirectback(id, EVENT_DATA);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendmessageto(id, &message);
        channel_sendstringzto(id, EVENT_QUERY, "data");

        while ((c = channel_readfrom(id, message.data.buffer, MESSAGE_SIZE)))
        {

            socket_bind_ipv4s(&remote, message.data.buffer);
            socket_bind_tcps(&remote, "6667", 0);
            channel_sendbuffer(EVENT_DATA, cstring_length(message.data.buffer), message.data.buffer);

        }

    }

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
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
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    resolve(domain);

    if (file_walk(FILE_G1, FILE_G0, "data"))
    {

        char *request = "NICK jfu_fudge\nUSER jfu_fudge 0 * :Jens Fudge\nJOIN #fudge\n";
        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_G1);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_connect_tcp(FILE_G1, &local, &remote, &router);
        socket_send_tcp(FILE_G1, &local, &remote, &router, cstring_length(request), request);

        while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_sendbuffer(EVENT_DATA, count, buffer);

        socket_disconnect_tcp(FILE_G1, &local, &remote, &router);
        file_unlink(FILE_G1);

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    if (cstring_match(key, "ethernet"))
        file_walk2(FILE_G0, value);

    if (cstring_match(key, "domain"))
        cstring_copy(domain, value);

    if (cstring_match(key, "local-address"))
        socket_bind_ipv4s(&local, value);

    if (cstring_match(key, "local-port"))
        socket_bind_tcps(&local, value, 42);

    if (cstring_match(key, "remote-address") || cstring_match(key, "address"))
        socket_bind_ipv4s(&remote, value);

    if (cstring_match(key, "remote-port") || cstring_match(key, "port"))
        socket_bind_tcps(&remote, value, 0);

    if (cstring_match(key, "router-address"))
        socket_bind_ipv4s(&router, value);

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "50003", 42);
    socket_init(&remote);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

