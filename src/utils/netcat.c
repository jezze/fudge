#include <abi.h>
#include <fudge.h>

static void send(struct ipv4_socket *sender, struct ipv4_socket *target, void *buffer, unsigned int count)
{

    struct {struct ctrl_conheader header; char buffer[FUDGE_BSIZE];} packet;

    memory_copy(&packet.header.sender, sender, sizeof (struct ipv4_socket));
    memory_copy(&packet.header.target, target, sizeof (struct ipv4_socket));

    packet.header.count = memory_write(packet.buffer, FUDGE_BSIZE, buffer, count, 0);

    file_writeall(FILE_L1, &packet, sizeof (struct ctrl_conheader) + packet.header.count);

}

void main(void)
{

    struct ctrl_conheader header;
    char buffer[FUDGE_BSIZE];
    struct ipv4_arpentry entry;
    struct ipv4_socket socket;

    if (!file_walk(FILE_L0, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &entry, sizeof (struct ipv4_arpentry));
    file_close(FILE_L0);

    socket.address[0] = entry.paddress[0];
    socket.address[1] = entry.paddress[1];
    socket.address[2] = entry.paddress[2];
    socket.address[3] = entry.paddress[3];
    socket.port[0] = 0x1F;
    socket.port[1] = 0x90;

    if (!file_walk(FILE_L0, "/system/con/con:0"))
        return;

    if (!file_walkfrom(FILE_L1, FILE_L0, "ctrl"))
        return;

    file_open(FILE_L1);
    file_writeall(FILE_L1, &socket, sizeof (struct ipv4_socket));
    file_close(FILE_L1);

    if (!file_walkfrom(FILE_L1, FILE_L0, "data"))
        return;

    file_open(FILE_PO);
    file_open(FILE_L1);

    while (file_readall(FILE_L1, &header, sizeof (struct ctrl_conheader)))
    {

        file_readall(FILE_L1, buffer, header.count);
        file_writeall(FILE_PO, buffer, header.count);
        send(&socket, &header.sender, "ack\n", 4);

    }

    file_close(FILE_L1);
    file_close(FILE_PO);

}

