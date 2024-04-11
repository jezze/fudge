#include "reg.h"
#include "lcd.h"

void lcd_setup(void)
{

    unsigned char *fb = (unsigned char *)0x200000;
    unsigned int i;

    reg_write32(0x10000014, 0xA05F);
    reg_write32(0x1000001C, 0x12C11);
    reg_write32(0xC0000000, 0x3F1F3F9C);
    reg_write32(0xC0000004, 0x080B61DF);
    reg_write32(0xC0000008, 0x067F3800);
    reg_write32(0xC0000010, 0x200000);
    reg_write32(0xC000001C, 0x182B);
    reg_write32(0x1000000C, 0x33805);

    for (i = 0; i < (800 * 600 * 3); i += 1)
    {

        fb[i + 0] = 0xFF;

    }

}

