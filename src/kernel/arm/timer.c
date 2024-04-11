#include "pic.h"
#include "reg.h"
#include "timer.h"

#define CTRL_SIZE_32            0x02
#define CTRL_INT_ENABLE         (1 << 5)
#define CTRL_MODE_PERIODIC      0x40
#define CTRL_ENABLE             0x80

void timer_setup(void)
{

    reg_write32(0x13000000, 0x00FFFFFF);
    reg_write32(0x13000008, CTRL_SIZE_32 | CTRL_INT_ENABLE | CTRL_MODE_PERIODIC | CTRL_ENABLE);
    reg_write32(0x1300000C, ~0);
    pic_enableirq(PIC_IRQ_TIMER0);

}

