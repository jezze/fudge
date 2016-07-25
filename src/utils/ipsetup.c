#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned char mac[6];
    unsigned char ip[4] = {10, 0, 5, 5};

    if (!call_walk(CALL_L0, CALL_PR, 25, "system/ethernet/if:0/addr"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 29, "system/ethernet/ipv4/arptable"))
        return;

    call_open(CALL_L0);
    file_read(CALL_L0, mac, 6);
    call_close(CALL_L0);
    call_open(CALL_L1);
    file_write(CALL_L1, mac, 6);
    file_write(CALL_L1, ip, 4);
    call_close(CALL_L1);

}

