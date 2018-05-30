#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ipv4_arpentry entry;

    if (!file_walk(FILE_L0, "/system/ethernet/if:0/addr"))
        return;

    file_open(FILE_L0);
    file_readall(FILE_L0, &entry.haddress, 6);
    file_close(FILE_L0);

    if (!file_walk(FILE_L0, "/system/ethernet/ipv4/arptable"))
        return;

    entry.paddress[0] = 10;
    entry.paddress[1] = 0;
    entry.paddress[2] = 5;
    entry.paddress[3] = entry.haddress[4];

    file_open(FILE_L0);
    file_writeall(FILE_L0, &entry, sizeof (struct ipv4_arpentry));
    file_close(FILE_L0);

}

