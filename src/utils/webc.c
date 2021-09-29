#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char url[512];

static unsigned int buildrequest(unsigned int count, void *buffer, struct url *kurl)
{

    unsigned int offset = 0;

    offset += buffer_write(buffer, count, "GET /", 5, offset);

    if (kurl->path)
        offset += buffer_write(buffer, count, kurl->path, ascii_length(kurl->path), offset);

    offset += buffer_write(buffer, count, " HTTP/1.1\r\n", 11, offset);
    offset += buffer_write(buffer, count, "Host: ", 6, offset);
    offset += buffer_write(buffer, count, kurl->host, ascii_length(kurl->host), offset);
    offset += buffer_write(buffer, count, "\r\n\r\n", 4, offset);

    return offset;

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

        while ((c = channel_readsource(id, message.data.buffer, MESSAGE_SIZE)))
            socket_bind_ipv4s(&remote, message.data.buffer);

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char urldata[BUFFER_SIZE];
    struct url kurl;

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (ascii_length(url) >= 4 && buffer_match(url, "http", 4))
        url_parse(&kurl, urldata, BUFFER_SIZE, url, URL_SCHEME);
    else
        url_parse(&kurl, urldata, BUFFER_SIZE, url, URL_HOST);

    if (kurl.host)
        resolve(kurl.host);

    if (kurl.port)
        socket_bind_tcps(&remote, kurl.port, 0);

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count = buildrequest(BUFFER_SIZE, buffer, &kurl);

        file_link(FILE_L0);
        socket_resolveremote(FILE_L0, &local, &router);
        socket_connect_tcp(FILE_L0, &local, &remote, &router);
        socket_send_tcp(FILE_L0, &local, &remote, &router, count, buffer);

        while ((count = socket_receive_tcp(FILE_L0, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_sendbuffer(EVENT_DATA, count, buffer);

        socket_disconnect_tcp(FILE_L0, &local, &remote, &router);
        file_unlink(FILE_L0);

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
        file_walk2(FILE_G0, value);

    if (ascii_match(key, "url"))
        ascii_copy(url, value);

    if (ascii_match(key, "local-address"))
        socket_bind_ipv4s(&local, value);

    if (ascii_match(key, "local-port"))
        socket_bind_tcps(&local, value, 42);

    if (ascii_match(key, "remote-address") || ascii_match(key, "address"))
        socket_bind_ipv4s(&remote, value);

    if (ascii_match(key, "remote-port") || ascii_match(key, "port"))
        socket_bind_tcps(&remote, value, 0);

    if (ascii_match(key, "router-address"))
        socket_bind_ipv4s(&router, value);

}

void init(void)
{

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_tcps(&local, "50001", 42);
    socket_init(&remote);
    socket_bind_tcps(&remote, "80", 0);
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

