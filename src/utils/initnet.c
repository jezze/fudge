#include <fudge.h>
#include <abi.h>

struct ipv4_arpentry
{

    unsigned char haddress[6];
    unsigned char paddress[4];

};

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct ipv4_arpentry entry;

    if (!file_walk2(FILE_L0, "/system/ethernet/if:0/addr"))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &entry.haddress, 6);
    file_close(FILE_L0);

    entry.paddress[0] = 10;
    entry.paddress[1] = 0;
    entry.paddress[2] = 5;
    entry.paddress[3] = entry.haddress[4];

    if (!file_walk2(FILE_L0, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(FILE_L0);
    file_writeall(FILE_L0, &entry, sizeof (struct ipv4_arpentry));
    file_close(FILE_L0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

