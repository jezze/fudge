#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char request[1024];
static unsigned int reqc = 0;

struct dns_header
{

    unsigned char id[2];
    unsigned char flags[2];
    unsigned char questions[2];
    unsigned char answers[2];
    unsigned char authority[2];
    unsigned char additional[2];

};

struct dns_question
{

    unsigned char type[2];
    unsigned char class[2];

};

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    /*
    char *key = mdata;
    char *value = key + ascii_length(key) + 1;
    */
    struct dns_header header;
    struct dns_question question;
    unsigned char namelen;

    buffer_clear(&header, sizeof (struct dns_header));
    buffer_clear(&question, sizeof (struct dns_question));

    net_save16(header.id, 0x0001);
    net_save16(header.flags, 0x0100);
    net_save16(header.questions, 0x0001);
    net_save16(question.type, 0x0001);
    net_save16(question.class, 0x0001);

    reqc += buffer_write(request, 1024, &header, sizeof (struct dns_header), reqc);

    namelen = 3;

    reqc += buffer_write(request, 1024, &namelen, 1, reqc);
    reqc += buffer_write(request, 1024, "www", namelen, reqc);

    namelen = 7;

    reqc += buffer_write(request, 1024, &namelen, 1, reqc);
    reqc += buffer_write(request, 1024, "blunder", namelen, reqc);

    namelen = 2;

    reqc += buffer_write(request, 1024, &namelen, 1, reqc);
    reqc += buffer_write(request, 1024, "se", namelen, reqc);

    reqc += buffer_write(request, 1024, "\0", 1, reqc);

/*
    reqc += buffer_write(request, 1024, value, ascii_lengthz(value), reqc);
*/

    reqc += buffer_write(request, 1024, &question, sizeof (struct dns_question), reqc);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char buffer[BUFFER_SIZE];
    unsigned int count;

    file_link(FILE_G0);
    socket_resolveremote(channel, FILE_G0, &local, &router);
    socket_send_udp(FILE_G0, &local, &remote, &router, reqc, request);

    while ((count = socket_receive_udp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE)))
        channel_reply(channel, EVENT_DATA, count, buffer);

    file_unlink(FILE_G0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {8, 8, 8, 8};
    unsigned char port2[TCP_PORTSIZE] = {0x00, 0x35};
    unsigned char address3[IPV4_ADDRSIZE] = {10, 0, 5, 80};

    if (!file_walk2(FILE_L0, "system:ethernet/if:0"))
        return;

    if (!file_walk(FILE_L1, FILE_L0, "addr"))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        return;

    socket_init(&local);
    socket_bind_ipv4(&local, address1);
    socket_bind_udp(&local, port1);
    socket_init(&remote);
    socket_bind_ipv4(&remote, address2);
    socket_bind_udp(&remote, port2);
    socket_init(&router);
    socket_bind_ipv4(&router, address3);
    socket_resolvelocal(FILE_L1, &local);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

