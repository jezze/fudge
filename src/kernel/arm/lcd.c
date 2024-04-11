#include "reg.h"
#include "lcd.h"

#define LCD_REG_TIMING0                 0xC0000000
#define LCD_REG_TIMING1                 0xC0000004
#define LCD_REG_TIMING2                 0xC0000008
#define LCD_REG_UPBASE                  0xC0000010
#define LCD_REG_LPBASE                  0xC0000014
#define LCD_REG_INTENABLE               0xC0000018
#define LCD_REG_CONTROL                 0xC000001C
#define LCD_REG_STATUS                  0xC0000020

void lcd_setup(void)
{

    unsigned char *fb = (unsigned char *)0x200000;
    unsigned int i;

    reg_write32(0x10000014, 0xA05F);
    reg_write32(0x1000001C, 0x12C11);
    reg_write32(LCD_REG_TIMING0, 0x3F1F3F9C);
    reg_write32(LCD_REG_TIMING1, 0x080B61DF);
    reg_write32(LCD_REG_TIMING2, 0x067F3800);
    reg_write32(LCD_REG_UPBASE, 0x200000);
    reg_write32(LCD_REG_CONTROL, 0x182B);
    reg_write32(0x1000000C, 0x33805);

    for (i = 0; i < (800 * 600 * 3); i += 1)
    {

        fb[i + 0] = 0xFF;

    }

}

