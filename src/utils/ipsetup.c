#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char mac[6];
    unsigned char ip[4] = {10, 0, 5, 5};

    if (!file_walk(CALL_L0, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk(CALL_L1, "/system/ethernet/ipv4/arptable"))
        return;

    file_open(CALL_L0);
    file_read(CALL_L0, mac, 6);
    file_close(CALL_L0);
    file_open(CALL_L1);
    file_write(CALL_L1, mac, 6);
    file_write(CALL_L1, ip, 4);
    file_close(CALL_L1);

}

