#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct {struct ipv4_pair pair; char buffer[FUDGE_BSIZE];} rpacket;
    struct {struct ipv4_pair pair; char buffer[FUDGE_BSIZE];} spacket;
    struct ipv4_arpentry entry;

    if (!file_walk(FILE_L1, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(FILE_L1);
    file_readall(FILE_L1, &entry, sizeof (struct ipv4_arpentry));
    file_close(FILE_L1);

    spacket.pair.sender.address[0] = entry.paddress[0];
    spacket.pair.sender.address[1] = entry.paddress[1];
    spacket.pair.sender.address[2] = entry.paddress[2];
    spacket.pair.sender.address[3] = entry.paddress[3];
    spacket.pair.sender.port[0] = 0x1F;
    spacket.pair.sender.port[1] = 0x90;
    spacket.pair.count = memory_write(spacket.buffer, FUDGE_BSIZE, "HELLO", 5, 0);

    if (!file_walk(FILE_L1, "/system/con/con:0"))
        return;

    if (!file_walkfrom(FILE_L2, FILE_L1, "ctrl"))
        return;

    if (!file_walkfrom(FILE_L3, FILE_L1, "data"))
        return;

    file_open(FILE_L2);
    file_writeall(FILE_L2, &spacket.pair.sender, sizeof (struct ipv4_socket));
    file_close(FILE_L2);
    file_open(FILE_PO);
    file_open(FILE_L3);

    while (file_readall(FILE_L3, &rpacket.pair, sizeof (struct ipv4_pair)))
    {

        file_readall(FILE_L3, &rpacket.buffer, rpacket.pair.count);
        file_writeall(FILE_PO, rpacket.buffer, rpacket.pair.count);
        memory_copy(&spacket.pair.target, &rpacket.pair.sender, sizeof (struct ipv4_socket));
        file_writeall(FILE_L3, &spacket, sizeof (struct ipv4_pair) + spacket.pair.count);

    }

    file_close(FILE_L3);
    file_close(FILE_PO);

}

