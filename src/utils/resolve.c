#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct socket local;

static void ondata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct socket remote;

    file_link(FILE_G0);
    socket_init(&remote);
    socket_bind(&remote, mdata);
    socket_resolveremote(channel, FILE_G0, &local, &remote);
    file_unlink(FILE_G0);
    channel_close(channel);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};

    if (!file_walk2(FILE_L0, "system:ethernet/if:0"))
        return;

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        return;

    if (!file_walk(FILE_G1, FILE_L0, "addr"))
        return;

    socket_init(&local);
    socket_bind(&local, address);
    socket_resolvelocal(FILE_G1, &local);
    channel_setcallback(channel, EVENT_DATA, ondata);

}

