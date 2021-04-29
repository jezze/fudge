#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;
static struct socket remote;
static struct socket router;
static char request[1024];
static unsigned int requestlength = 0;

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

struct dns_answer
{

    unsigned char type[2];
    unsigned char class[2];
    unsigned char ttl[4];
    unsigned char rdlength[2];

};

static unsigned int copyname(void *buffer, unsigned int count, char *name)
{

    unsigned char namelen = 0;
    unsigned int valstart = 0;
    unsigned int c = 0;
    unsigned int i;

    for (i = 0; i < ascii_lengthz(name); i++)
    {

        if (name[i] == '.' || name[i] == '\0')
        {

            c += buffer_write(buffer, count, &namelen, 1, c);
            c += buffer_write(buffer, count, name + valstart, namelen, c);

            valstart = i + 1;
            namelen = 0;

        }

        else
        {

            namelen++;

        }

    }

    c += buffer_write(buffer, count, "\0", 1, c);

    return c;

}

static unsigned int putname(void *buffer, unsigned int count, char *name, void *start)
{

    unsigned int offset = 0;
    unsigned int index = 0;

    while (name[index])
    {

        if ((name[index] & 0xC0) == 0xC0)
        {

            unsigned char *temp = start;

            name = (char *)(temp + (((name[index] << 8) & 0x2F) | name[index + 1]));
            index = 0;

        }

        else
        {

            offset += buffer_write(buffer, count, name + index + 1, name[index], offset);
            index += name[index] + 1;

            if (name[index])
                offset += buffer_write(buffer, count, ".", 1, offset);

        }

    }

    return offset;

}

static unsigned int namesize(void *buffer)
{

    char *name = buffer;
    unsigned int index = 0;

    while (name[index])
    {

        if ((name[index] & 0xC0) == 0xC0)
            return index + 2;
        else
            index += name[index] + 1;

    }

    return index + 1;

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;

    if (ascii_match(key, "name"))
    {

        char *value = key + ascii_lengthz(key);
        struct dns_header header;
        struct dns_question question;

        buffer_clear(&header, sizeof (struct dns_header));
        buffer_clear(&question, sizeof (struct dns_question));
        net_save16(header.id, 0x0001);
        net_save16(header.flags, 0x0100);
        net_save16(header.questions, 0x0001);
        net_save16(question.type, 0x0001);
        net_save16(question.class, 0x0001);

        requestlength += buffer_write(request, 1024, &header, sizeof (struct dns_header), requestlength);
        requestlength += copyname(request + requestlength, 1024 - requestlength, value);
        requestlength += buffer_write(request, 1024, &question, sizeof (struct dns_question), requestlength);

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (requestlength > 0)
    {

        unsigned char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_G0);
        socket_resolveremote(channel, FILE_G0, &local, &router);
        socket_send_udp(FILE_G0, &local, &remote, &router, requestlength, request);

        count = socket_receive_udp(channel, FILE_G0, &local, &remote, &router, buffer, BUFFER_SIZE);

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
                responselength += namesize(name);
                responselength += sizeof (struct dns_question);

            }

            for (i = 0; i < net_load16(header->answers); i++)
            {

                struct dns_answer *answer;
                char *name;
                unsigned char *addr;
                char temp[256];

                name = (char *)(buffer + responselength);
                responselength += namesize(name);
                answer = (struct dns_answer *)(buffer + responselength);
                responselength += sizeof (struct dns_answer);
                addr = (unsigned char *)(buffer + responselength);
                responselength += net_load16(answer->rdlength);
                offset = message_putstring(&data, "Type: 0x", offset);
                offset = message_putvalue(&data, net_load16(answer->type), 16, 4, offset);
                offset = message_putstring(&data, "\n", offset);
                offset = message_putstring(&data, "Name: ", offset);
                offset = message_putbuffer(&data, putname(temp, 256, name, buffer), temp, offset);
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

            }

            channel_reply(channel, EVENT_DATA, offset, &data);

        }

        file_unlink(FILE_G0);

    }

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

