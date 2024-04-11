#include "reg.h"
#include "pic.h"
#include "kmi.h"

#include "uart.h"

#define KMI_REG_KBD_CONTROL             0x18000000
#define KMI_REG_KBD_STATUS              0x18000004
#define KMI_REG_KBD_DATA                0x18000008
#define KMI_REG_KBD_CLOCK               0x1800000C
#define KMI_REG_KBD_INTERRUPT           0x18000010
#define KMI_REG_MOUSE_CONTROL           0x19000000
#define KMI_REG_MOUSE_STATUS            0x19000004
#define KMI_REG_MOUSE_DATA              0x19000008
#define KMI_REG_MOUSE_CLOCK             0x1900000C
#define KMI_REG_MOUSE_INTERRUPT         0x19000010

#define KMI_CONTROL_ENABLE              (1 << 2)
#define KMI_CONTROL_INTTX               (1 << 3)
#define KMI_CONTROL_INTRX               (1 << 4)

void kmi_kbd_irq(void)
{

    unsigned int value = reg_read32(KMI_REG_KBD_DATA);

    uart_puts("KEYBOARD\n");

    if (value == 0)
        uart_puts("IGNORE\n");

}

void kmi_mouse_irq(void)
{

    unsigned int value = reg_read32(KMI_REG_MOUSE_DATA);

    uart_puts("MOUSE\n");

    if (value == 0)
        uart_puts("IGNORE\n");

}

void kmi_setup(void)
{

    reg_write32(KMI_REG_KBD_CONTROL, KMI_CONTROL_ENABLE | KMI_CONTROL_INTRX);
    pic_enableirq(PIC_IRQ_KEYBOARD);
    /*
    reg_write32(KMI_REG_MOUSE_CONTROL, KMI_CONTROL_ENABLE | KMI_CONTROL_INTRX);
    pic_enableirq(PIC_IRQ_MOUSE);
    */

}

