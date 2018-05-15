#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct {struct ipv4_pair pair; char buffer[FUDGE_BSIZE];} rpacket;
    struct {struct ipv4_pair pair; char buffer[FUDGE_BSIZE];} spacket;
    struct ctrl_consettings consettings;

    if (!file_walk(FILE_L1, "/system/con/con:0"))
        return;

    if (!file_walkfrom(FILE_L2, FILE_L1, "ctrl"))
        return;

    if (!file_walkfrom(FILE_L3, FILE_L1, "data"))
        return;

    consettings.interface = 0;
    consettings.port = 8080;

    file_open(FILE_L2);
    file_writeall(FILE_L2, &consettings, sizeof (struct ctrl_consettings));
    file_close(FILE_L2);
    file_open(FILE_PO);
    file_open(FILE_L3);
    file_writeall(FILE_PO, "IP: 10.0.5.5\n", 13);
    file_writeall(FILE_PO, "Protocol: UDP\n", 14);
    file_writeall(FILE_PO, "Port: 8080\n", 11);
    file_writeall(FILE_PO, "Listening...\n\n", 14);

    while (file_readall(FILE_L3, &rpacket.pair, sizeof (struct ipv4_pair)))
    {

        file_readall(FILE_L3, &rpacket.buffer, rpacket.pair.count);
        file_writeall(FILE_PO, rpacket.buffer, rpacket.pair.count);
        memory_copy(&spacket.pair.target, &rpacket.pair.sender, sizeof (struct ipv4_address));
        memory_copy(&spacket.pair.sender, &rpacket.pair.target, sizeof (struct ipv4_address));

        spacket.pair.count = memory_write(spacket.buffer, FUDGE_BSIZE, "HELLO", 5, 0);

        file_writeall(FILE_L3, &spacket, sizeof (struct ipv4_pair) + spacket.pair.count);

    }

    file_close(FILE_L3);
    file_close(FILE_PO);

}

