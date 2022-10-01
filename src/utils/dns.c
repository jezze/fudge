#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct option options[32];
static struct socket local;
static struct socket remote;
static struct socket router;

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
    offset += dns_copyname((char *)buffer + offset, count - offset, option_getstring(options, "domain"));
    offset += buffer_write(buffer, count, &question, sizeof (struct dns_question), offset);

    return offset;

}

static void reply(unsigned short type, char *name, void *rddata, void *buffer)
{

    unsigned char *addr = rddata;
    struct message message;
    char fullname[256];

    message_init(&message, EVENT_DATA);

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

    socket_bind_ipv4s(&local, option_getstring(options, "local-address"));
    socket_bind_udps(&local, option_getstring(options, "local-port"));
    socket_bind_ipv4s(&remote, option_getstring(options, "remote-address"));
    socket_bind_udps(&remote, option_getstring(options, "remote-port"));
    socket_bind_ipv4s(&router, option_getstring(options, "router-address"));

    if (!file_walk2(FILE_L0, option_getstring(options, "ethernet")))
        channel_warning("Could not open ethernet");

    if (file_walk(FILE_L1, FILE_L0, "addr"))
        socket_resolvelocal(FILE_L1, &local);

    if (file_walk(FILE_L1, FILE_L0, "data"))
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count = buildrequest(BUFFER_SIZE, buffer);

        file_link(FILE_L1);
        socket_resolveremote(FILE_L1, &local, &router);
        socket_send_udp(FILE_L1, &local, &remote, &router, count, buffer);

        count = socket_receive_udp(FILE_L1, &local, &remote, &router, buffer, BUFFER_SIZE);

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

        file_unlink(FILE_L1);

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
    socket_init(&remote);
    socket_init(&router);
    option_add(options, "ethernet", "system:ethernet/if:0");
    option_add(options, "domain", "");
    option_add(options, "local-address", "10.0.5.1");
    option_add(options, "local-port", "50000");
    option_add(options, "remote-address", "8.8.8.8");
    option_add(options, "remote-port", "53");
    option_add(options, "router-address", "10.0.5.80");
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

