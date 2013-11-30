enum vga_register
{

    VGA_REGISTER_CRTINDEX0              = 0x03B4,
    VGA_REGISTER_CRTDATA0               = 0x03B5,
    VGA_REGISTER_FCMCTRL                = 0x03BA,
    VGA_REGISTER_ARINDEX                = 0x03C0,
    VGA_REGISTER_ARWRITE                = 0x03C0,
    VGA_REGISTER_ARREAD                 = 0x03C1,
    VGA_REGISTER_MISCWRITE              = 0x03C2,
    VGA_REGISTER_SRINDEX                = 0x03C4,
    VGA_REGISTER_SRDATA                 = 0x03C5,
    VGA_REGISTER_DACPIXMASK             = 0x03C6,
    VGA_REGISTER_DACRINDEX              = 0x03C7,
    VGA_REGISTER_DACWINDEX              = 0x03C8,
    VGA_REGISTER_DACDATA                = 0x03C9,
    VGA_REGISTER_FCSTAT                 = 0x03CA,
    VGA_REGISTER_MISCREAD               = 0x03CC,
    VGA_REGISTER_GRINDEX                = 0x03CE,
    VGA_REGISTER_GRDATA                 = 0x03CF,
    VGA_REGISTER_CRTINDEX1              = 0x03D4,
    VGA_REGISTER_CRTDATA1               = 0x03D5,
    VGA_REGISTER_FCCCTRL                = 0x03DA

};

enum vga_arindex
{

    VGA_ARINDEX_AR00                    = 0x00,
    VGA_ARINDEX_AR01                    = 0x01,
    VGA_ARINDEX_AR02                    = 0x02,
    VGA_ARINDEX_AR03                    = 0x03,
    VGA_ARINDEX_AR04                    = 0x04,
    VGA_ARINDEX_AR05                    = 0x05,
    VGA_ARINDEX_AR06                    = 0x06,
    VGA_ARINDEX_AR07                    = 0x07,
    VGA_ARINDEX_AR08                    = 0x08,
    VGA_ARINDEX_AR09                    = 0x09,
    VGA_ARINDEX_AR0A                    = 0x0A,
    VGA_ARINDEX_AR0B                    = 0x0B,
    VGA_ARINDEX_AR0C                    = 0x0C,
    VGA_ARINDEX_AR0D                    = 0x0D,
    VGA_ARINDEX_AR0E                    = 0x0E,
    VGA_ARINDEX_AR0F                    = 0x0F,
    VGA_ARINDEX_AR10                    = 0x10,
    VGA_ARINDEX_AR11                    = 0x11,
    VGA_ARINDEX_AR12                    = 0x12,
    VGA_ARINDEX_AR13                    = 0x13,
    VGA_ARINDEX_AR14                    = 0x14,
    VGA_ARINDEX_DISABLE                 = (0 << 5),
    VGA_ARINDEX_ENABLE                  = (1 << 5)

};

enum vga_ar10
{

    VGA_AR10_GRAPHICS                   = (1 << 0),
    VGA_AR10_MDA                        = (1 << 1),
    VGA_AR10_LINE                       = (1 << 2),
    VGA_AR10_BLINK                      = (1 << 3),
    VGA_AR10_PIXPAN                     = (1 << 5),
    VGA_AR10_PIXCLK                     = (1 << 6),
    VGA_AR10_ENABLE14                   = (1 << 7)

};

enum vga_ar12
{

    VGA_AR12_ENABLE0                    = (1 << 0),
    VGA_AR12_ENABLE1                    = (1 << 1),
    VGA_AR12_ENABLE2                    = (1 << 2),
    VGA_AR12_ENABLE3                    = (1 << 3)

};

enum vga_crtindex
{

    VGA_CRTINDEX_CRT00                  = 0x00,
    VGA_CRTINDEX_CRT01                  = 0x01,
    VGA_CRTINDEX_CRT02                  = 0x02,
    VGA_CRTINDEX_CRT03                  = 0x03,
    VGA_CRTINDEX_CRT04                  = 0x04,
    VGA_CRTINDEX_CRT05                  = 0x05,
    VGA_CRTINDEX_CRT06                  = 0x06,
    VGA_CRTINDEX_CRT07                  = 0x07,
    VGA_CRTINDEX_CRT08                  = 0x08,
    VGA_CRTINDEX_CRT09                  = 0x09,
    VGA_CRTINDEX_CRT0A                  = 0x0A,
    VGA_CRTINDEX_CRT0B                  = 0x0B,
    VGA_CRTINDEX_CRT0C                  = 0x0C,
    VGA_CRTINDEX_CRT0D                  = 0x0D,
    VGA_CRTINDEX_CRT0E                  = 0x0E,
    VGA_CRTINDEX_CRT0F                  = 0x0F,
    VGA_CRTINDEX_CRT10                  = 0x10,
    VGA_CRTINDEX_CRT11                  = 0x11,
    VGA_CRTINDEX_CRT12                  = 0x12,
    VGA_CRTINDEX_CRT13                  = 0x13,
    VGA_CRTINDEX_CRT14                  = 0x14,
    VGA_CRTINDEX_CRT15                  = 0x15,
    VGA_CRTINDEX_CRT16                  = 0x16,
    VGA_CRTINDEX_CRT17                  = 0x17,
    VGA_CRTINDEX_CRT18                  = 0x18,
    VGA_CRTINDEX_CRT22                  = 0x22,
    VGA_CRTINDEX_CRT24                  = 0x24,
    VGA_CRTINDEX_CRT26                  = 0x26

};

enum vga_crt03
{

    VGA_CRT03_EVRA                      = (1 << 7)

};

enum vga_crt09
{

    VGA_CRT09_SD                        = (1 << 7)

};

enum vga_crt0A
{

    VGA_CRT0A_CD                        = (1 << 5)

};

enum vga_crt11
{

    VGA_CRT11_BANDWIDTH                 = (1 << 6),
    VGA_CRT11_PROTECT                   = (1 << 7)

};

enum vga_crt14
{

    VGA_CRT14_DIV4                      = (1 << 5),
    VGA_CRT14_DW                        = (1 << 6)

};

enum vga_crt17
{

    VGA_CRT17_MAP13                     = (1 << 0),
    VGA_CRT17_MAP14                     = (1 << 1),
    VGA_CRT17_SLDIV                     = (1 << 2),
    VGA_CRT17_DIV2                      = (1 << 3),
    VGA_CRT17_AW                        = (1 << 5),
    VGA_CRT17_WORD                      = (1 << 6),
    VGA_CRT17_SE                        = (1 << 7)

};

enum vga_grindex
{

    VGA_GRINDEX_GR00                    = 0x00,
    VGA_GRINDEX_GR01                    = 0x01,
    VGA_GRINDEX_GR02                    = 0x02,
    VGA_GRINDEX_GR03                    = 0x03,
    VGA_GRINDEX_GR04                    = 0x04,
    VGA_GRINDEX_GR05                    = 0x05,
    VGA_GRINDEX_GR06                    = 0x06,
    VGA_GRINDEX_GR07                    = 0x07,
    VGA_GRINDEX_GR08                    = 0x08

};

enum vga_gr03
{

    VGA_GR03_NONE                       = (0 << 3),
    VGA_GR03_AND                        = (1 << 3),
    VGA_GR03_OR                         = (2 << 3),
    VGA_GR03_XOR                        = (3 << 3)

};

enum vga_gr05
{

    VGA_GR05_WMODE0                     = (0 << 0),
    VGA_GR05_WMODE1                     = (1 << 0),
    VGA_GR05_WMODE2                     = (2 << 0),
    VGA_GR05_WMODE3                     = (3 << 0),
    VGA_GR05_RMODE0                     = (0 << 3),
    VGA_GR05_RMODE1                     = (1 << 3),
    VGA_GR05_ODDEVEN                    = (1 << 4),
    VGA_GR05_SHIFTMODE                  = (1 << 5),
    VGA_GR05_256MODE                    = (1 << 6)

};

enum vga_gr06
{

    VGA_GR06_GRAPHICS                   = (1 << 0),
    VGA_GR06_CHAIN                      = (1 << 1),
    VGA_GR06_EXTENDED                   = (0 << 2),
    VGA_GR06_EGAVGA                     = (1 << 2),
    VGA_GR06_HERCULES                   = (2 << 2),
    VGA_GR06_CGA                        = (3 << 2)

};

enum vga_misc
{

    VGA_MISC_MONO                       = (0 << 0),
    VGA_MISC_COLOR                      = (1 << 0),
    VGA_MISC_DISABLE                    = (0 << 1),
    VGA_MISC_ENABLE                     = (1 << 1),
    VGA_MISC_VCLK0                      = (0 << 2),
    VGA_MISC_VCLK1                      = (1 << 2),
    VGA_MISC_VCLK2                      = (2 << 2),
    VGA_MISC_VCLK3                      = (3 << 2),
    VGA_MISC_PAGESELECT                 = (1 << 5),
    VGA_MISC_SYNC400                    = (1 << 6),
    VGA_MISC_SYNC350                    = (2 << 6),
    VGA_MISC_SYNC480                    = (3 << 6)

};

enum vga_srindex
{

    VGA_SRINDEX_SR00                    = 0x00,
    VGA_SRINDEX_SR01                    = 0x01,
    VGA_SRINDEX_SR02                    = 0x02,
    VGA_SRINDEX_SR03                    = 0x03,
    VGA_SRINDEX_SR04                    = 0x04

};

enum vga_sr00
{

    VGA_SR00_ASYNC                      = (1 << 0),
    VGA_SR00_SYNC                       = (1 << 1)

};

enum vga_sr01
{

    VGA_SR01_DIV9                       = (0 << 0),
    VGA_SR01_DIV8                       = (1 << 0),
    VGA_SR01_SHIFT16                    = (1 << 2),
    VGA_SR01_DIV2                       = (1 << 3),
    VGA_SR01_SHIFT32                    = (1 << 4),
    VGA_SR01_FULL                       = (1 << 5)

};

enum vga_sr02
{

    VGA_SR02_ENABLE0                    = (1 << 0),
    VGA_SR02_ENABLE1                    = (1 << 1),
    VGA_SR02_ENABLE2                    = (1 << 2),
    VGA_SR02_ENABLE3                    = (1 << 3)

};

enum vga_sr03
{

    VGA_SR03_P0K                        = (0 << 0),
    VGA_SR03_P16K                       = (1 << 0),
    VGA_SR03_P32K                       = (2 << 0),
    VGA_SR03_P48K                       = (3 << 0),
    VGA_SR03_P8K                        = ((1 << 4) | VGA_SR03_P0K),
    VGA_SR03_P24K                       = ((1 << 4) | VGA_SR03_P16K),
    VGA_SR03_P40K                       = ((1 << 4) | VGA_SR03_P32K),
    VGA_SR03_P56K                       = ((1 << 4) | VGA_SR03_P48K),
    VGA_SR03_S0K                        = (0 << 2),
    VGA_SR03_S16K                       = (1 << 2),
    VGA_SR03_S32K                       = (2 << 2),
    VGA_SR03_S48K                       = (3 << 2),
    VGA_SR03_S8K                        = ((1 << 5) | VGA_SR03_S0K),
    VGA_SR03_S24K                       = ((1 << 5) | VGA_SR03_S16K),
    VGA_SR03_S40K                       = ((1 << 5) | VGA_SR03_S32K),
    VGA_SR03_S56K                       = ((1 << 5) | VGA_SR03_S48K)

};

enum vga_sr04
{

    VGA_SR04_EXTENDED                   = (1 << 1),
    VGA_SR04_ODDEVEN                    = (1 << 2),
    VGA_SR04_CHAIN                      = (1 << 3)

};

unsigned int inar(unsigned char index);
unsigned int incrt1(unsigned char index);
unsigned int insr(unsigned char index);
void outar(unsigned char index, unsigned char value);
void outcrt1(unsigned char index, unsigned char value);
void outgr(unsigned char index, unsigned char value);
void outsr(unsigned char index, unsigned char value);
