#define VGA_ADDRESS                     0x000A0000
#define VGA_TEXT_ADDRESS                0x000B8000
#define VGA_TEXT_SIZE                   2000

#define VGA_REG_CR_MONO_INDEX           0x03B4
#define VGA_REG_CR_MONO_DATA            0x03B5
#define VGA_REG_FC_MONO_CTRL            0x03BA
#define VGA_REG_AR_INDEX                0x03C0
#define VGA_REG_AR_DATA                 0x03C1
#define VGA_REG_MISC_CTRL               0x03C2
#define VGA_REG_SR_INDEX                0x03C4
#define VGA_REG_SR_DATA                 0x03C5
#define VGA_REG_DAC_PIXMASK             0x03C6
#define VGA_REG_DAC_STATUS              0x03C7
#define VGA_REG_DAC_CTRL                0x03C8
#define VGA_REG_DAC_DATA                0x03C9
#define VGA_REG_FC_STATUS               0x03CA
#define VGA_REG_MISC_DATA               0x03CC
#define VGA_REG_GR_INDEX                0x03CE
#define VGA_REG_GR_DATA                 0x03CF
#define VGA_REG_CR_COLOR_INDEX          0x03D4
#define VGA_REG_CR_COLOR_DATA           0x03D5
#define VGA_REG_FC_COLOR_CTRL           0x03DA

#define VGA_INDEX_AR00                  0x00
#define VGA_INDEX_AR01                  0x01
#define VGA_INDEX_AR02                  0x02
#define VGA_INDEX_AR03                  0x03
#define VGA_INDEX_AR04                  0x04
#define VGA_INDEX_AR05                  0x05
#define VGA_INDEX_AR06                  0x06
#define VGA_INDEX_AR07                  0x07
#define VGA_INDEX_AR08                  0x08
#define VGA_INDEX_AR09                  0x09
#define VGA_INDEX_AR0A                  0x0A
#define VGA_INDEX_AR0B                  0x0B
#define VGA_INDEX_AR0C                  0x0C
#define VGA_INDEX_AR0D                  0x0D
#define VGA_INDEX_AR0E                  0x0E
#define VGA_INDEX_AR0F                  0x0F
#define VGA_INDEX_AR10                  0x10
#define VGA_INDEX_AR11                  0x11
#define VGA_INDEX_AR12                  0x12
#define VGA_INDEX_AR13                  0x13
#define VGA_INDEX_AR14                  0x14

#define VGA_INDEX_CR00                  0x00
#define VGA_INDEX_CR01                  0x01
#define VGA_INDEX_CR02                  0x02
#define VGA_INDEX_CR03                  0x03
#define VGA_INDEX_CR04                  0x04
#define VGA_INDEX_CR05                  0x05
#define VGA_INDEX_CR06                  0x06
#define VGA_INDEX_CR07                  0x07
#define VGA_INDEX_CR08                  0x08
#define VGA_INDEX_CR09                  0x09
#define VGA_INDEX_CR0A                  0x0A
#define VGA_INDEX_CR0B                  0x0B
#define VGA_INDEX_CR0C                  0x0C
#define VGA_INDEX_CR0D                  0x0D
#define VGA_INDEX_CR0E                  0x0E
#define VGA_INDEX_CR0F                  0x0F
#define VGA_INDEX_CR10                  0x10
#define VGA_INDEX_CR11                  0x11
#define VGA_INDEX_CR12                  0x12
#define VGA_INDEX_CR13                  0x13
#define VGA_INDEX_CR14                  0x14
#define VGA_INDEX_CR15                  0x15
#define VGA_INDEX_CR16                  0x16
#define VGA_INDEX_CR17                  0x17
#define VGA_INDEX_CR18                  0x18
#define VGA_INDEX_CR22                  0x22
#define VGA_INDEX_CR24                  0x24
#define VGA_INDEX_CR26                  0x26

#define VGA_INDEX_GR00                  0x00
#define VGA_INDEX_GR01                  0x01
#define VGA_INDEX_GR02                  0x02
#define VGA_INDEX_GR03                  0x03
#define VGA_INDEX_GR04                  0x04
#define VGA_INDEX_GR05                  0x05
#define VGA_INDEX_GR06                  0x06
#define VGA_INDEX_GR07                  0x07
#define VGA_INDEX_GR08                  0x08

#define VGA_INDEX_SR00                  0x00
#define VGA_INDEX_SR01                  0x01
#define VGA_INDEX_SR02                  0x02
#define VGA_INDEX_SR03                  0x03
#define VGA_INDEX_SR04                  0x04

#define VGA_AR_ENABLE                   (1 << 5)

#define VGA_AR10_GRAPHICS               (1 << 0)
#define VGA_AR10_MDA                    (1 << 1)
#define VGA_AR10_LINE                   (1 << 2)
#define VGA_AR10_BLINK                  (1 << 3)
#define VGA_AR10_PIXPAN                 (1 << 5)
#define VGA_AR10_PIXCLK                 (1 << 6)
#define VGA_AR10_ENABLE14               (1 << 7)

#define VGA_MISC_MONO                   (0 << 0)
#define VGA_MISC_COLOR                  (1 << 0)
#define VGA_MISC_DISABLE                (0 << 1)
#define VGA_MISC_ENABLE                 (1 << 1)
#define VGA_MISC_VCLK0                  (0 << 2)
#define VGA_MISC_VCLK1                  (1 << 2)
#define VGA_MISC_VCLK2                  (2 << 2)
#define VGA_MISC_VCLK3                  (3 << 2)
#define VGA_MISC_PAGESELECT             (1 << 5)
#define VGA_MISC_SYNC400                (1 << 6)
#define VGA_MISC_SYNC350                (2 << 6)
#define VGA_MISC_SYNC480                (3 << 6)

#define VGA_GR3_NONE                    (0 << 3)
#define VGA_GR3_AND                     (1 << 3)
#define VGA_GR3_OR                      (2 << 3)
#define VGA_GR3_XOR                     (3 << 3)

#define VGA_GR5_WMODE0                  (0 << 0)
#define VGA_GR5_WMODE1                  (1 << 0)
#define VGA_GR5_WMODE2                  (2 << 0)
#define VGA_GR5_WMODE3                  (3 << 0)
#define VGA_GR5_RMODE0                  (0 << 3)
#define VGA_GR5_RMODE1                  (1 << 3)
#define VGA_GR5_ODDEVEN                 (1 << 4)
#define VGA_GR5_SHIFTMODE               (1 << 5)
#define VGA_GR5_256MODE                 (1 << 6)

#define VGA_GR6_GRAPHICS                (1 << 0)
#define VGA_GR6_CHAIN                   (1 << 1)
#define VGA_GR6_EXTENDED                (0 << 2)
#define VGA_GR6_EGAVGA                  (1 << 2)
#define VGA_GR6_HERCULES                (2 << 2)
#define VGA_GR6_CGA                     (3 << 2)

#define VGA_SR0_ASYNC                   (1 << 0)
#define VGA_SR0_SYNC                    (1 << 1)

#define VGA_SR1_DIV9                    (0 << 0)
#define VGA_SR1_DIV8                    (1 << 0)
#define VGA_SR1_SHIFT16                 (1 << 2)
#define VGA_SR1_DIV2                    (1 << 3)
#define VGA_SR1_SHIFT32                 (1 << 4)
#define VGA_SR1_FULL                    (1 << 5)

#define VGA_SR2_ENABLE0                 (1 << 0)
#define VGA_SR2_ENABLE1                 (1 << 1)
#define VGA_SR2_ENABLE2                 (1 << 2)
#define VGA_SR2_ENABLE3                 (1 << 3)

#define VGA_SR3_P0K                     (0 << 0)
#define VGA_SR3_P16K                    (1 << 0)
#define VGA_SR3_P32K                    (2 << 0)
#define VGA_SR3_P48K                    (3 << 0)
#define VGA_SR3_P8K                     ((1 << 4) | VGA_SR3_P0K)
#define VGA_SR3_P24K                    ((1 << 4) | VGA_SR3_P16K)
#define VGA_SR3_P40K                    ((1 << 4) | VGA_SR3_P32K)
#define VGA_SR3_P56K                    ((1 << 4) | VGA_SR3_P48K)
#define VGA_SR3_S0K                     (0 << 2)
#define VGA_SR3_S16K                    (1 << 2)
#define VGA_SR3_S32K                    (2 << 2)
#define VGA_SR3_S48K                    (3 << 2)
#define VGA_SR3_S8K                     ((1 << 5) | VGA_SR3_S0K)
#define VGA_SR3_S24K                    ((1 << 5) | VGA_SR3_S16K)
#define VGA_SR3_S40K                    ((1 << 5) | VGA_SR3_S32K)
#define VGA_SR3_S56K                    ((1 << 5) | VGA_SR3_S48K)

#define VGA_SR4_EXTENDED                (1 << 1)
#define VGA_SR4_ODDEVEN                 (1 << 2)
#define VGA_SR4_CHAIN                   (1 << 3)

struct vga_cursor
{

    unsigned char color;
    unsigned short offset;

};

struct vga_driver
{

    struct base_driver base;
    struct video_interface video;
    struct terminal_interface terminal;
    struct vga_cursor cursor;

};

void vga_init_driver(struct vga_driver *driver);
