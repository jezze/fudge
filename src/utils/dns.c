#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char name[512];

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
    offset += dns_copyname((char *)buffer + offset, count - offset, name);
    offset += buffer_write(buffer, count, &question, sizeof (struct dns_question), offset);

    return offset;

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    unsigned char buffer[BUFFER_SIZE];
    unsigned int count = buildrequest(BUFFER_SIZE, buffer);

    if (file_walk(FILE_L0, FILE_G0, "addr"))
        socket_resolvelocal(FILE_L0, &local);

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        file_link(FILE_L0);
        socket_resolveremote(channel, FILE_L0, &local, &router);
        socket_send_udp(FILE_L0, &local, &remote, &router, count, buffer);

        count = socket_receive_udp(channel, FILE_L0, &local, &remote, &router, buffer, BUFFER_SIZE);

        if (count)
        {

            struct dns_header *header = (struct dns_header *)buffer;
            unsigned int responselength = sizeof (struct dns_header);
            struct message_data data;
            unsigned int offset = 0;
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
                unsigned char *addr;
                char temp[256];

                /* temp stuff */
                char *xa = "address";
                char *xv = "255.255.255.255";
                char xmsg[32];
                unsigned int xc = 0;

                name = (char *)(buffer + responselength);
                responselength += dns_namesize(name);
                answer = (struct dns_answer *)(buffer + responselength);
                responselength += sizeof (struct dns_answer);
                addr = (unsigned char *)(buffer + responselength);
                responselength += net_load16(answer->rdlength);
                offset = message_putstring(&data, "Type: 0x", offset);
                offset = message_putvalue(&data, net_load16(answer->type), 16, 4, offset);
                offset = message_putstring(&data, "\n", offset);
                offset = message_putstring(&data, "Name: ", offset);
                offset = message_putbuffer(&data, dns_writename(temp, 256, name, buffer), temp, offset);
                offset = message_putstring(&data, "\n", offset);

                if (net_load16(answer->type) == 0x0001)
                {

                    offset = message_putstring(&data, "Address: ", offset);
                    offset = message_putvalue(&data, addr[0], 10, 0, offset);
                    offset = message_putstring(&data, ".", offset);
                    offset = message_putvalue(&data, addr[1], 10, 0, offset);
                    offset = message_putstring(&data, ".", offset);
                    offset = message_putvalue(&data, addr[2], 10, 0, offset);
                    offset = message_putstring(&data, ".", offset);
                    offset = message_putvalue(&data, addr[3], 10, 0, offset);
                    offset = message_putstring(&data, "\n", offset);

                }

                xc += buffer_write(xmsg, 32, xa, ascii_lengthz(xa), xc);
                xc += buffer_write(xmsg, 32, xv, ascii_lengthz(xv), xc);

                channel_reply(channel, EVENT_OPTION, xc, xmsg);

            }

            channel_reply(channel, EVENT_DATA, offset, &data);

        }

        file_unlink(FILE_L0);

    }

    channel_close(channel);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
    {

        file_walk2(FILE_G0, value);

    }

    if (ascii_match(key, "name"))
    {

        ascii_copy(name, value);

    }

}

void init(struct channel *channel)
{

    unsigned char address1[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port1[TCP_PORTSIZE] = {0x07, 0xD0};
    unsigned char address2[IPV4_ADDRSIZE] = {8, 8, 8, 8};
    unsigned char port2[TCP_PORTSIZE] = {0x00, 0x35};
    unsigned char address3[IPV4_ADDRSIZE] = {10, 0, 5, 80};

    file_walk2(FILE_G0, "system:ethernet/if:0");
    socket_init(&local);
    socket_bind_ipv4(&local, address1);
    socket_bind_udp(&local, port1);
    socket_init(&remote);
    socket_bind_ipv4(&remote, address2);
    socket_bind_udp(&remote, port2);
    socket_init(&router);
    socket_bind_ipv4(&router, address3);
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

