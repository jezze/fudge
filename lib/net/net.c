#include "net.h"

unsigned char net_load8(unsigned char data[1])
{

    return data[0];

}

unsigned short net_load16(unsigned char data[2])
{

    return (data[0] << 8) | (data[1] << 0);

}

unsigned int net_load32(unsigned char data[4])
{

    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);

}

void net_save8(unsigned char data[1], unsigned char num)
{

    data[0] = num >> 0;

}

void net_save16(unsigned char data[2], unsigned short num)
{

    data[0] = num >> 8;
    data[1] = num >> 0;

}

void net_save32(unsigned char data[4], unsigned int num)
{

    data[0] = num >> 24;
    data[1] = num >> 16;
    data[2] = num >> 8;
    data[3] = num >> 0;

}

