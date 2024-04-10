#include "reg.h"
#include "pic.h"
#include "kmi.h"

void kmi_setup(void)
{

    reg_write32(KMI_REG_KBD_CONTROL, (1 << 2) | (1 << 4));
    pic_enableirq(PIC_IRQ_KEYBOARD);

}

