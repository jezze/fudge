#define VGA_REG_CRTINDEX0                   0x03B4
#define VGA_REG_CRTDATA0                    0x03B5
#define VGA_REG_FCMCTRL                     0x03BA
#define VGA_REG_ARINDEX                     0x03C0
#define VGA_REG_ARINDEX_AR00                0x00
#define VGA_REG_ARINDEX_AR01                0x01
#define VGA_REG_ARINDEX_AR02                0x02
#define VGA_REG_ARINDEX_AR03                0x03
#define VGA_REG_ARINDEX_AR04                0x04
#define VGA_REG_ARINDEX_AR05                0x05
#define VGA_REG_ARINDEX_AR06                0x06
#define VGA_REG_ARINDEX_AR07                0x07
#define VGA_REG_ARINDEX_AR08                0x08
#define VGA_REG_ARINDEX_AR09                0x09
#define VGA_REG_ARINDEX_AR0A                0x0A
#define VGA_REG_ARINDEX_AR0B                0x0B
#define VGA_REG_ARINDEX_AR0C                0x0C
#define VGA_REG_ARINDEX_AR0D                0x0D
#define VGA_REG_ARINDEX_AR0E                0x0E
#define VGA_REG_ARINDEX_AR0F                0x0F
#define VGA_REG_ARINDEX_AR10                0x10
#define VGA_REG_ARINDEX_AR10_GRAPHICS       (1 << 0)
#define VGA_REG_ARINDEX_AR10_MDA            (1 << 1)
#define VGA_REG_ARINDEX_AR10_LINE           (1 << 2)
#define VGA_REG_ARINDEX_AR10_BLINK          (1 << 3)
#define VGA_REG_ARINDEX_AR10_PIXPAN         (1 << 5)
#define VGA_REG_ARINDEX_AR10_PIXCLK         (1 << 6)
#define VGA_REG_ARINDEX_AR10_ENABLE14       (1 << 7)
#define VGA_REG_ARINDEX_AR11                0x11
#define VGA_REG_ARINDEX_AR12                0x12
#define VGA_REG_ARINDEX_AR12_ENABLE0        (1 << 0)
#define VGA_REG_ARINDEX_AR12_ENABLE1        (1 << 1)
#define VGA_REG_ARINDEX_AR12_ENABLE2        (1 << 2)
#define VGA_REG_ARINDEX_AR12_ENABLE3        (1 << 3)
#define VGA_REG_ARINDEX_AR13                0x13
#define VGA_REG_ARINDEX_AR14                0x14
#define VGA_REG_ARINDEX_DISABLE             (0 << 5)
#define VGA_REG_ARINDEX_ENABLE              (1 << 5)
#define VGA_REG_ARWRITE                     0x03C0
#define VGA_REG_ARREAD                      0x03C1
#define VGA_REG_MISCWRITE                   0x03C2
#define VGA_REG_MISCWRITE_MONO              (0 << 0)
#define VGA_REG_MISCWRITE_COLOR             (1 << 0)
#define VGA_REG_MISCWRITE_DISABLE           (0 << 1)
#define VGA_REG_MISCWRITE_ENABLE            (1 << 1)
#define VGA_REG_MISCWRITE_VCLK0             (0 << 2)
#define VGA_REG_MISCWRITE_VCLK1             (1 << 2)
#define VGA_REG_MISCWRITE_VCLK2             (2 << 2)
#define VGA_REG_MISCWRITE_VCLK3             (3 << 2)
#define VGA_REG_MISCWRITE_PAGESELECT        (1 << 5)
#define VGA_REG_MISCWRITE_SYNC400           (1 << 6)
#define VGA_REG_MISCWRITE_SYNC350           (2 << 6)
#define VGA_REG_MISCWRITE_SYNC480           (3 << 6)
#define VGA_REG_SRINDEX                     0x03C4
#define VGA_REG_SRINDEX_SR00                0x00
#define VGA_REG_SRINDEX_SR00_ASYNC          (1 << 0)
#define VGA_REG_SRINDEX_SR00_SYNC           (1 << 1)
#define VGA_REG_SRINDEX_SR01                0x01
#define VGA_REG_SRINDEX_SR01_DIV9           (0 << 0)
#define VGA_REG_SRINDEX_SR01_DIV8           (1 << 0)
#define VGA_REG_SRINDEX_SR01_SHIFT16        (1 << 2)
#define VGA_REG_SRINDEX_SR01_DIV2           (1 << 3)
#define VGA_REG_SRINDEX_SR01_SHIFT32        (1 << 4)
#define VGA_REG_SRINDEX_SR01_FULL           (1 << 5)
#define VGA_REG_SRINDEX_SR02                0x02
#define VGA_REG_SRINDEX_SR02_ENABLE0        (1 << 0)
#define VGA_REG_SRINDEX_SR02_ENABLE1        (1 << 1)
#define VGA_REG_SRINDEX_SR02_ENABLE2        (1 << 2)
#define VGA_REG_SRINDEX_SR02_ENABLE3        (1 << 3)
#define VGA_REG_SRINDEX_SR03                0x03
#define VGA_REG_SRINDEX_SR03_P0K            (0 << 0)
#define VGA_REG_SRINDEX_SR03_P16K           (1 << 0)
#define VGA_REG_SRINDEX_SR03_P32K           (2 << 0)
#define VGA_REG_SRINDEX_SR03_P48K           (3 << 0)
#define VGA_REG_SRINDEX_SR03_P8K            ((1 << 4) | VGA_REG_SRINDEX_SR03_P0K)
#define VGA_REG_SRINDEX_SR03_P24K           ((1 << 4) | VGA_REG_SRINDEX_SR03_P16K)
#define VGA_REG_SRINDEX_SR03_P40K           ((1 << 4) | VGA_REG_SRINDEX_SR03_P32K)
#define VGA_REG_SRINDEX_SR03_P56K           ((1 << 4) | VGA_REG_SRINDEX_SR03_P48K)
#define VGA_REG_SRINDEX_SR03_S0K            (0 << 2)
#define VGA_REG_SRINDEX_SR03_S16K           (1 << 2)
#define VGA_REG_SRINDEX_SR03_S32K           (2 << 2)
#define VGA_REG_SRINDEX_SR03_S48K           (3 << 2)
#define VGA_REG_SRINDEX_SR03_S8K            ((1 << 5) | VGA_REG_SRINDEX_SR03_S0K)
#define VGA_REG_SRINDEX_SR03_S24K           ((1 << 5) | VGA_REG_SRINDEX_SR03_S16K)
#define VGA_REG_SRINDEX_SR03_S40K           ((1 << 5) | VGA_REG_SRINDEX_SR03_S32K)
#define VGA_REG_SRINDEX_SR03_S56K           ((1 << 5) | VGA_REG_SRINDEX_SR03_S48K)
#define VGA_REG_SRINDEX_SR04                0x04
#define VGA_REG_SRINDEX_SR04_EXTENDED       (1 << 1)
#define VGA_REG_SRINDEX_SR04_ODDEVEN        (1 << 2)
#define VGA_REG_SRINDEX_SR04_CHAIN          (1 << 3)
#define VGA_REG_SRDATA                      0x03C5
#define VGA_REG_DACPIXMASK                  0x03C6
#define VGA_REG_DACRINDEX                   0x03C7
#define VGA_REG_DACWINDEX                   0x03C8
#define VGA_REG_DACDATA                     0x03C9
#define VGA_REG_FCSTAT                      0x03CA
#define VGA_REG_MISCREAD                    0x03CC
#define VGA_REG_GRINDEX                     0x03CE
#define VGA_REG_GRINDEX_GR00                0x00
#define VGA_REG_GRINDEX_GR01                0x01
#define VGA_REG_GRINDEX_GR02                0x02
#define VGA_REG_GRINDEX_GR03                0x03
#define VGA_REG_GRINDEX_GR03_NONE           (0 << 3)
#define VGA_REG_GRINDEX_GR03_AND            (1 << 3)
#define VGA_REG_GRINDEX_GR03_OR             (2 << 3)
#define VGA_REG_GRINDEX_GR03_XOR            (3 << 3)
#define VGA_REG_GRINDEX_GR04                0x04
#define VGA_REG_GRINDEX_GR05                0x05
#define VGA_REG_GRINDEX_GR05_WMODE0         (0 << 0)
#define VGA_REG_GRINDEX_GR05_WMODE1         (1 << 0)
#define VGA_REG_GRINDEX_GR05_WMODE2         (2 << 0)
#define VGA_REG_GRINDEX_GR05_WMODE3         (3 << 0)
#define VGA_REG_GRINDEX_GR05_RMODE0         (0 << 3)
#define VGA_REG_GRINDEX_GR05_RMODE1         (1 << 3)
#define VGA_REG_GRINDEX_GR05_ODDEVEN        (1 << 4)
#define VGA_REG_GRINDEX_GR05_SHIFTMODE      (1 << 5)
#define VGA_REG_GRINDEX_GR05_256MODE        (1 << 6)
#define VGA_REG_GRINDEX_GR06                0x06
#define VGA_REG_GRINDEX_GR07                0x07
#define VGA_REG_GRINDEX_GR08                0x08
#define VGA_REG_GRINDEX_GR06_GRAPHICS       (1 << 0)
#define VGA_REG_GRINDEX_GR06_CHAIN          (1 << 1)
#define VGA_REG_GRINDEX_GR06_EXTENDED       (0 << 2)
#define VGA_REG_GRINDEX_GR06_EGAVGA         (1 << 2)
#define VGA_REG_GRINDEX_GR06_HERCULES       (2 << 2)
#define VGA_REG_GRINDEX_GR06_CGA            (3 << 2)
#define VGA_REG_GRDATA                      0x03CF
#define VGA_REG_CRTINDEX1                   0x03D4
#define VGA_REG_CRTINDEX1_CRT00             0x00
#define VGA_REG_CRTINDEX1_CRT01             0x01
#define VGA_REG_CRTINDEX1_CRT02             0x02
#define VGA_REG_CRTINDEX1_CRT03             0x03
#define VGA_REG_CRTINDEX1_CRT03_EVRA        (1 << 7)
#define VGA_REG_CRTINDEX1_CRT04             0x04
#define VGA_REG_CRTINDEX1_CRT05             0x05
#define VGA_REG_CRTINDEX1_CRT06             0x06
#define VGA_REG_CRTINDEX1_CRT07             0x07
#define VGA_REG_CRTINDEX1_CRT08             0x08
#define VGA_REG_CRTINDEX1_CRT09             0x09
#define VGA_REG_CRTINDEX1_CRT09_SD          (1 << 7)
#define VGA_REG_CRTINDEX1_CRT0A             0x0A
#define VGA_REG_CRTINDEX1_CRT0A_CD          (1 << 5)
#define VGA_REG_CRTINDEX1_CRT0B             0x0B
#define VGA_REG_CRTINDEX1_CRT0C             0x0C
#define VGA_REG_CRTINDEX1_CRT0D             0x0D
#define VGA_REG_CRTINDEX1_CRT0E             0x0E
#define VGA_REG_CRTINDEX1_CRT0F             0x0F
#define VGA_REG_CRTINDEX1_CRT10             0x10
#define VGA_REG_CRTINDEX1_CRT11             0x11
#define VGA_REG_CRTINDEX1_CRT11_BANDWIDTH   (1 << 6)
#define VGA_REG_CRTINDEX1_CRT11_PROTECT     (1 << 7)
#define VGA_REG_CRTINDEX1_CRT12             0x12
#define VGA_REG_CRTINDEX1_CRT13             0x13
#define VGA_REG_CRTINDEX1_CRT14             0x14
#define VGA_REG_CRTINDEX1_CRT14_DIV4        (1 << 5)
#define VGA_REG_CRTINDEX1_CRT14_DW          (1 << 6)
#define VGA_REG_CRTINDEX1_CRT15             0x15
#define VGA_REG_CRTINDEX1_CRT16             0x16
#define VGA_REG_CRTINDEX1_CRT17             0x17
#define VGA_REG_CRTINDEX1_CRT17_MAP13       (1 << 0)
#define VGA_REG_CRTINDEX1_CRT17_MAP14       (1 << 1)
#define VGA_REG_CRTINDEX1_CRT17_SLDIV       (1 << 2)
#define VGA_REG_CRTINDEX1_CRT17_DIV2        (1 << 3)
#define VGA_REG_CRTINDEX1_CRT17_AW          (1 << 5)
#define VGA_REG_CRTINDEX1_CRT17_WORD        (1 << 6)
#define VGA_REG_CRTINDEX1_CRT17_SE          (1 << 7)
#define VGA_REG_CRTINDEX1_CRT18             0x18
#define VGA_REG_CRTINDEX1_CRT22             0x22
#define VGA_REG_CRTINDEX1_CRT24             0x24
#define VGA_REG_CRTINDEX1_CRT26             0x26
#define VGA_REG_CRTDATA1                    0x03D5
#define VGA_REG_FCCCTRL                     0x03DA

unsigned int inar(unsigned char index);
unsigned int incrt1(unsigned char index);
unsigned int insr(unsigned char index);
void outar(unsigned char index, unsigned char value);
void outcrt1(unsigned char index, unsigned char value);
void outgr(unsigned char index, unsigned char value);
void outsr(unsigned char index, unsigned char value);
void vga_save(void);
void vga_restore(void);
void vga_settext(void);
void vga_setgraphic(void);
void vga_setmode(unsigned int flags, unsigned int vdisplay, unsigned int vscan, unsigned int clockindex, unsigned int depth, unsigned int htotal, unsigned int hdisplay, unsigned int hsyncstart, unsigned int hsyncend, unsigned int hskew, unsigned int vtotal, unsigned int vsyncstart, unsigned int vsyncend, unsigned int vblankstart, unsigned int vblankend, unsigned int linewidth);
