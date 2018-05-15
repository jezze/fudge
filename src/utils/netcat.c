#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_conheader header;
    char buffer[FUDGE_BSIZE];
    struct {struct ctrl_conheader header; char buffer[FUDGE_BSIZE];} packet;
    struct ipv4_arpentry entry;

    if (!file_walk(FILE_L1, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(FILE_L1);
    file_readall(FILE_L1, &entry, sizeof (struct ipv4_arpentry));
    file_close(FILE_L1);

    packet.header.sender.address[0] = entry.paddress[0];
    packet.header.sender.address[1] = entry.paddress[1];
    packet.header.sender.address[2] = entry.paddress[2];
    packet.header.sender.address[3] = entry.paddress[3];
    packet.header.sender.port[0] = 0x1F;
    packet.header.sender.port[1] = 0x90;
    packet.header.count = memory_write(packet.buffer, FUDGE_BSIZE, "HELLO", 5, 0);

    if (!file_walk(FILE_L1, "/system/con/con:0"))
        return;

    if (!file_walkfrom(FILE_L2, FILE_L1, "ctrl"))
        return;

    file_open(FILE_L2);
    file_writeall(FILE_L2, &packet.header.sender, sizeof (struct ipv4_socket));
    file_close(FILE_L2);

    if (!file_walkfrom(FILE_L2, FILE_L1, "data"))
        return;

    file_open(FILE_PO);
    file_open(FILE_L2);

    while (file_readall(FILE_L2, &header, sizeof (struct ctrl_conheader)))
    {

        file_readall(FILE_L2, buffer, header.count);
        file_writeall(FILE_PO, buffer, header.count);
        memory_copy(&packet.header.target, &header.sender, sizeof (struct ipv4_socket));
        file_writeall(FILE_L2, &packet, sizeof (struct ctrl_conheader) + packet.header.count);

    }

    file_close(FILE_L2);
    file_close(FILE_PO);

}

