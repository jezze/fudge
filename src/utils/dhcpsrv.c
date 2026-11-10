#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

#define PARAMETERS                      3

struct lease
{

    unsigned int xid;
    unsigned char address[IPV4_ADDRSIZE];
    unsigned char netmask[IPV4_ADDRSIZE];
    unsigned char route[IPV4_ADDRSIZE];
    unsigned char dns[IPV4_ADDRSIZE];
    unsigned char server[IPV4_ADDRSIZE];

};

static struct socket local;
static struct socket server;
static struct socket broadcast;

static unsigned int putoption(void *buffer, unsigned int count, unsigned int offset, unsigned char code, unsigned char length, void *data)
{

    unsigned char header[2];

    header[0] = code;
    header[1] = length;

    return buffer_write(buffer, count, header, 2, offset) + buffer_write(buffer, count, data, length, offset + 2);

}

static unsigned int buildmessage(void *buffer, unsigned int count, unsigned char type, struct lease *lease)
{

    unsigned char parameters[PARAMETERS];
    unsigned char end = DHCP_OPTION_END;
    struct dhcp_header header;
    unsigned int offset = 0;

    parameters[0] = DHCP_OPTION_NETMASK;
    parameters[1] = DHCP_OPTION_ROUTER;
    parameters[2] = DHCP_OPTION_DNS;

    buffer_clear(&header, sizeof (struct dhcp_header));
    net_save8(header.op, DHCP_OP_REQUEST);
    net_save8(header.htype, 1);
    net_save8(header.hlen, ETHERNET_ADDRSIZE);
    net_save32(header.xid, lease->xid);
    net_save16(header.flags, DHCP_FLAGS_BROADCAST);
    buffer_copy(header.chaddr, local.haddress, ETHERNET_ADDRSIZE);
    net_save32(header.magic, DHCP_MAGIC);

    offset += buffer_write(buffer, count, &header, sizeof (struct dhcp_header), offset);
    offset += putoption(buffer, count, offset, DHCP_OPTION_TYPE, 1, &type);

    if (type == DHCP_REQUEST)
    {

        offset += putoption(buffer, count, offset, DHCP_OPTION_REQUESTEDADDRESS, IPV4_ADDRSIZE, lease->address);
        offset += putoption(buffer, count, offset, DHCP_OPTION_SERVER, IPV4_ADDRSIZE, lease->server);

    }

    offset += putoption(buffer, count, offset, DHCP_OPTION_PARAMETERS, PARAMETERS, parameters);
    offset += buffer_write(buffer, count, &end, 1, offset);

    if (offset < DHCP_MINSIZE && DHCP_MINSIZE <= count)
    {

        buffer_clear((unsigned char *)buffer + offset, DHCP_MINSIZE - offset);

        offset = DHCP_MINSIZE;

    }

    return offset;

}

static unsigned int parsemessage(struct lease *lease, unsigned char *data, unsigned int count)
{

    struct dhcp_header *header = (struct dhcp_header *)data;
    unsigned int offset = sizeof (struct dhcp_header);
    unsigned int type = 0;

    if (count < offset || net_load8(header->op) != DHCP_OP_REPLY || net_load32(header->xid) != lease->xid || net_load32(header->magic) != DHCP_MAGIC)
        return 0;

    buffer_copy(lease->address, header->yiaddr, IPV4_ADDRSIZE);

    while (offset < count)
    {

        unsigned char code = net_load8(data + offset);
        unsigned char *value = data + offset + 2;
        unsigned char length;

        if (code == DHCP_OPTION_END)
            break;

        if (code == DHCP_OPTION_PAD)
        {

            offset++;

            continue;

        }

        if (offset + 2 > count)
            break;

        length = net_load8(data + offset + 1);

        if (offset + 2 + length > count)
            break;

        switch (code)
        {

        case DHCP_OPTION_TYPE:
            if (length == 1)
                type = net_load8(value);

            break;

        case DHCP_OPTION_NETMASK:
            if (length >= IPV4_ADDRSIZE)
                buffer_copy(lease->netmask, value, IPV4_ADDRSIZE);

            break;

        case DHCP_OPTION_ROUTER:
            if (length >= IPV4_ADDRSIZE)
                buffer_copy(lease->route, value, IPV4_ADDRSIZE);

            break;

        case DHCP_OPTION_DNS:
            if (length >= IPV4_ADDRSIZE)
                buffer_copy(lease->dns, value, IPV4_ADDRSIZE);

            break;

        case DHCP_OPTION_SERVER:
            if (length >= IPV4_ADDRSIZE)
                buffer_copy(lease->server, value, IPV4_ADDRSIZE);

            break;

        }

        offset += 2 + length;

    }

    return type;

}

static unsigned int exchange(unsigned int ethernet, unsigned char type, struct lease *lease)
{

    unsigned char buffer[4096];
    unsigned int count;

    socket_send_udp(0, ethernet, &local, &broadcast, &broadcast, buildmessage(buffer, 4096, type, lease), buffer);

    do
    {

        socket_init(&server);

        count = socket_receive(0, ethernet, &local, &server, 1, &broadcast, buffer, 4096);

        if (count)
        {

            unsigned int reply = parsemessage(lease, buffer, count);

            if (reply)
                return reply;

        }

    } while (count);

    return 0;

}

static unsigned int writeaddress(char *data, unsigned int size, unsigned int count, char *key, unsigned char address[IPV4_ADDRSIZE])
{

    return cstring_write_fmt(data, size, count, "      %s: %c.%c.%c.%c\n", key, &address[0], &address[1], &address[2], &address[3]);

}

static unsigned int writelease(char *data, unsigned int size, char *device, struct lease *lease)
{

    unsigned int count = 0;

    count += cstring_write_fmt(data, size, count, "{\n  networks: [\n    {\n      device: %s\n      type: dynamic\n", device);
    count += writeaddress(data, size, count, "address", lease->address);
    count += writeaddress(data, size, count, "route", lease->route);
    count += writeaddress(data, size, count, "netmask", lease->netmask);
    count += writeaddress(data, size, count, "dns", lease->dns);
    count += cstring_write_fmt(data, size, count, "    }\n  ]\n}\n");

    return count;

}

static unsigned int savefile(char *path, char *data, unsigned int count)
{

    unsigned int target = fs_auth(path);
    unsigned int parent = (target) ? fs_walkparent(1, target, path) : 0;
    unsigned int id = (parent) ? fs_walk(1, target, 0, path) : 0;

    if (parent && (!id || fs_remove(1, target, parent, id)))
    {

        char *name = path + fs_dirlength(path);

        id = fs_create(1, target, parent, name, cstring_length(name));

        return id && fs_write_all(1, target, id, data, count, 0) == count;

    }

    return 0;

}

static void onmain(struct message *message)
{

    char *device = option_getstring("ethernet-service");
    unsigned int ethernet = channel_lookup(device);
    char type[16];

    if (!cstring_match(system_address(device, "type", "type", type, 16), "dynamic"))
        return;

    if (ethernet)
    {

        unsigned char haddress[ETHERNET_ADDRSIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        struct mtwist_state state;
        struct lease lease;
        unsigned int reply;

        buffer_clear(&lease, sizeof (struct lease));
        mtwist_seed1(&state, system_unixtime(option_getstring("clock-service")));

        lease.xid = mtwist_rand(&state);

        socket_bind_ipv4s(&local, "0.0.0.0");
        socket_bind_udpv(&local, DHCP_CLIENTPORT);
        socket_bind_ipv4s(&broadcast, "255.255.255.255");
        socket_bind_udpv(&broadcast, DHCP_SERVERPORT);
        buffer_copy(broadcast.haddress, haddress, ETHERNET_ADDRSIZE);
        socket_resolvelocal(0, ethernet, &local);
        channel_send(0, ethernet, MESSAGE_TYPE_LINK, 0, 0);

        reply = exchange(ethernet, DHCP_DISCOVER, &lease);

        if (reply == DHCP_OFFER)
            reply = exchange(ethernet, DHCP_REQUEST, &lease);

        channel_send(0, ethernet, MESSAGE_TYPE_UNLINK, 0, 0);

        if (reply == DHCP_ACK)
        {

            char data[512];

            if (savefile(SYSTEM_LEASE, data, writelease(data, 512, device, &lease)))
                channel_send_fmt(0, message->source, MESSAGE_TYPE_DATA, "%s: %c.%c.%c.%c\n", device, &lease.address[0], &lease.address[1], &lease.address[2], &lease.address[3]);
            else
                channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Could not save lease: %s\n", SYSTEM_LEASE);

        }

        else
        {

            channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "No lease for %s\n", device);

        }

    }

    else
    {

        channel_send_fmt(0, message->source, MESSAGE_TYPE_ERROR, "Ethernet service not found: %s\n", device);

    }

}

void init(void)
{

    socket_init(&local);
    socket_init(&server);
    socket_init(&broadcast);
    option_add("clock-service", "clock:0");
    option_add("ethernet-service", "ethernet:0");
    option_add("type", "");
    channel_bind(MESSAGE_TYPE_MAIN, onmain);

}

