#include <fudge.h>
#include "io.h"
#include "pic.h"

#define REG_COMMAND0                    0x0020
#define REG_COMMAND1                    0x00A0
#define REG_COMMAND_CONFIG              0x11
#define REG_DATA0                       0x0021
#define REG_DATA1                       0x00A1
#define REG_DATA_8086                   0x01
#define REG_DATA_VECTOR0                0x20
#define REG_DATA_VECTOR1                0x28

static void setchip(unsigned char command, unsigned char data, unsigned char vector, unsigned char wire)
{

    io_outb(command, REG_COMMAND_CONFIG);
    io_outb(data, vector);
    io_outb(data, wire);
    io_outb(data, REG_DATA_8086);

}

static void setmask(unsigned short port, unsigned char mask)
{

    io_outb(port, mask);

}

void pic_init(void)
{

    setchip(REG_COMMAND0, REG_DATA0, REG_DATA_VECTOR0, 0x04);
    setchip(REG_COMMAND1, REG_DATA1, REG_DATA_VECTOR1, 0x02);
    setmask(REG_DATA0, 0xFF);
    setmask(REG_DATA1, 0xFF);

}

