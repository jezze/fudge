#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char domain[512];

static unsigned int buildrequest(unsigned int count, void *buffer)
{

    struct dns_header header;
    struct dns_question question;
    unsigned int offset = 0;

    buffer_clear(&header, sizeof (struct dns_header));
    buffer_clear(&question, sizeof (struct dns_question));
    net_save16(header.id, 0x0001);
    net_save16(header.flags, 0x0100);
    net_save16(header.questions, 0x0001);
    net_save16(question.type, 0x0001);
    net_save16(question.class, 0x0001);

    offset += buffer_write(buffer, count, &header, sizeof (struct dns_header), offset);
    offset += dns_copyname((char *)buffer + offset, count - offset, domain);
    offset += buffer_write(buffer, count, &question, sizeof (struct dns_question), offset);

    return offset;

}

static void reply(unsigned short type, char *name, void *rddata, void *buffer)
{

    unsigned char *addr = rddata;
    struct message message;
    char fullname[256];

    message_init(&message, EVENT_MAIN);

    switch (type)
    {

    case 1:
        message_putbuffer(&message, dns_writename(fullname, 256, name, buffer), fullname);
        message_putstring(&message, " has address ");
        message_putvalue(&message, addr[0], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[1], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[2], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[3], 10, 0);
        message_putstring(&message, "\n");

        break;

    case 5:
        message_putbuffer(&message, dns_writename(fullname, 256, name, buffer), fullname);
        message_putstring(&message, " is an alias for ");
        message_putbuffer(&message, dns_writename(fullname, 256, rddata, buffer), fullname);
        message_putstring(&message, "\n");

        break;

    }

    channel_sendmessage(&message);

    message_init(&message, EVENT_QUERY);
    message_putstringz(&message, "type");
    message_putvalue(&message, type, 10, 0);
    message_putstringz(&message, "");
    channel_sendmessage(&message);

    message_init(&message, EVENT_QUERY);
    message_putstringz(&message, "name");
    message_putbuffer(&message, dns_writename(fullname, 256, name, buffer), fullname);
    message_putstringz(&message, "");
    channel_sendmessage(&message);

    message_init(&message, EVENT_QUERY);
    message_putstringz(&message, "data");

    switch (type)
    {

    case 1:
        message_putvalue(&message, addr[0], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[1], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[2], 10, 0);
        message_putstring(&message, ".");
        message_putvalue(&message, addr[3], 10, 0);
        message_putstringz(&message, "");

        break;

    case 5:
        message_putbuffer(&message, dns_writename(fullname, 256, rddata, buffer), fullname);
        message_putstringz(&message, "");

        break;

    default:
        message_putstringz(&message, "<null>");

        break;

    }

    channel_sendmessage(&message);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count = buildrequest(BUFFER_SIZE, buffer);

        file_link(FILE_L0);
        socket_resolveremote(FILE_L0, &local, &router);
        socket_send_udp(FILE_L0, &local, &remote, &router, count, buffer);

        count = socket_receive_udp(FILE_L0, &local, &remote, &router, buffer, BUFFER_SIZE);

        if (count)
        {

            struct dns_header *header = (struct dns_header *)buffer;
            unsigned int responselength = sizeof (struct dns_header);
            unsigned int i;

            for (i = 0; i < net_load16(header->questions); i++)
            {

                char *name;

                name = (char *)(buffer + responselength);
                responselength += dns_namesize(name);
                responselength += sizeof (struct dns_question);

            }

            for (i = 0; i < net_load16(header->answers); i++)
            {

                struct dns_answer *answer;
                char *name;
                void *rddata;

                name = (char *)(buffer + responselength);
                responselength += dns_namesize(name);
                answer = (struct dns_answer *)(buffer + responselength);
                responselength += sizeof (struct dns_answer);
                rddata = (buffer + responselength);
                responselength += net_load16(answer->rdlength);

                reply(net_load16(answer->type), name, rddata, buffer);

            }

        }

        file_unlink(FILE_L0);

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
        socket_bind_udps(&local, value);

    if (cstring_match(key, "remote-address") || cstring_match(key, "address"))
        socket_bind_ipv4s(&remote, value);

    if (cstring_match(key, "remote-port") || cstring_match(key, "port"))
        socket_bind_udps(&remote, value);

    if (cstring_match(key, "router-address"))
        socket_bind_ipv4s(&router, value);

}

void init(void)
{

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4s(&local, "10.0.5.1");
    socket_bind_udps(&local, "50000");
    socket_init(&remote);
    socket_bind_ipv4s(&remote, "8.8.8.8");
    socket_bind_udps(&remote, "53");
    socket_init(&router);
    socket_bind_ipv4s(&router, "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

