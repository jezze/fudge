#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

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
    offset += dns_copyname((char *)buffer + offset, count - offset, option_getstring("domain"));
    offset += buffer_write(buffer, count, &question, sizeof (struct dns_question), offset);

    return offset;

}

static void reply(unsigned short type, char *name, void *rddata, void *buffer)
{

    char fullname[256];
    unsigned int fullnamelength = dns_writename(fullname, 256, name, buffer);

    if (type == 1)
    {

        unsigned char *addr = rddata;

        channel_send_fmt6(CHANNEL_DEFAULT, EVENT_QUERY, "type\\01\\0name\\0%w\\0data\\0%c.%c.%c.%c\\0", fullname, &fullnamelength, &addr[0], &addr[1], &addr[2], &addr[3]);
        channel_send_fmt6(CHANNEL_DEFAULT, EVENT_DATA, "%w has address %c.%c.%c.%c\n", fullname, &fullnamelength, &addr[0], &addr[1], &addr[2], &addr[3]);

    }

    else if (type == 5)
    {

        char alias[256];
        unsigned int aliaslength = dns_writename(alias, 256, rddata, buffer);

        channel_send_fmt4(CHANNEL_DEFAULT, EVENT_QUERY, "type\\05\\0name\\0%w\\0data\\0%w\\0", fullname, &fullnamelength, alias, &aliaslength);
        channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "%w is an alias for %w\n", fullname, &fullnamelength, alias, &aliaslength);

    }

}

static void seed(struct mtwist_state *state)
{

    struct ctrl_clocksettings settings;

    if (!call_walk_absolute(FILE_L0, option_getstring("clock")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "ctrl"))
        PANIC();

    call_read_all(FILE_L1, &settings, sizeof (struct ctrl_clocksettings), 0);
    mtwist_seed1(state, time_unixtime(settings.year, settings.month, settings.day, settings.hours, settings.minutes, settings.seconds));

}

static void setupnetwork(struct mtwist_state *state)
{

    if (!call_walk_absolute(FILE_L0, option_getstring("ethernet")))
        PANIC();

    if (!call_walk_relative(FILE_L1, FILE_L0, "addr"))
        PANIC();

    if (!call_walk_relative(FILE_G0, FILE_L0, "data"))
        PANIC();

    socket_bind_ipv4s(&local, option_getstring("local-address"));
    socket_bind_udpv(&local, mtwist_rand(state));
    socket_bind_ipv4s(&remote, option_getstring("remote-address"));
    socket_bind_udpv(&remote, option_getdecimal("remote-port"));
    socket_bind_ipv4s(&router, option_getstring("router-address"));
    socket_resolvelocal(FILE_L1, &local);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char buffer[4096];
    unsigned int count;
    struct mtwist_state state;

    seed(&state);
    setupnetwork(&state);
    call_link(FILE_G0, 8000);
    socket_resolveremote(FILE_G0, &local, &router);
    socket_send_udp(FILE_G0, &local, &remote, &router, buildrequest(4096, buffer), buffer);

    count = socket_receive(FILE_G0, &local, &remote, 1, &router, buffer, 4096);

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

    call_unlink(FILE_G0);

}

void init(void)
{

    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add("clock", "system:clock/if.0");
    option_add("ethernet", "system:ethernet/if.0");
    option_add("local-address", "10.0.5.1");
    option_add("remote-address", "8.8.8.8");
    option_add("remote-port", "53");
    option_add("router-address", "10.0.5.80");
    option_add("domain", "");
    channel_bind(EVENT_MAIN, onmain);

}

