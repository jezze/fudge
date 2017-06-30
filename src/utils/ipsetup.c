#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char mac[6];
    unsigned char ip[4] = {10, 0, 5, 5};

    if (!file_walk(FILE_L0, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk(FILE_L1, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(FILE_L0);
    file_read(FILE_L0, mac, 6);
    file_close(FILE_L0);
    file_open(FILE_L1);
    file_write(FILE_L1, mac, 6);
    file_write(FILE_L1, ip, 4);
    file_close(FILE_L1);

}

