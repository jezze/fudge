#include <fudge/module.h>
#include <base/base.h>
#include <base/terminal.h>
#include <base/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "vga.h"
#include "timing.h"

#define VGA_ADDRESS                     0x000A0000
#define VGA_TEXT                        0x000B8000

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

/* BIOS mode 0Dh - 320x200x16 */
static const unsigned char g320x200x16[60] = {
    0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x14, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x09, 0x0F, 0x00, 0x06,
    0x63
};

/* BIOS mode 0Eh - 640x200x16 */
static const unsigned char g640x200x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};

/* BIOS mode 10h - 640x350x16 */
static const unsigned char g640x350x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x28, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA3
};

/* BIOS mode 12h - 640x480x16 */
static const unsigned char g640x480x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};

/* BIOS mode 13h - 320x200x256 */
static const unsigned char g320x200x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    0x63
};

/* non-BIOS mode - 320x240x256 */
static const unsigned char g320x240x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0D, 0x3E, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x28, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};

/* non-BIOS mode - 320x400x256 */
static const unsigned char g320x400x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};

/* non-BIOS mode - 360x480x256 */
static const unsigned char g360x480x256[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0x0D, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x2D, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE7
};

/* non BIOS mode - 720x348x2 based on mode 10h */
static const unsigned char g720x348x2[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x2D, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};

/* non-BIOS mode - 400x300x256 */
static const unsigned char g400x300x256X[60] = {
    0x71, 0x63, 0x64, 0x92, 0x65, 0x82, 0x46, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x31, 0x80, 0x2B, 0x32, 0x00, 0x2F, 0x44, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};

static void outar(unsigned char index, unsigned char value)
{

    io_outb(VGA_REGISTER_ARINDEX, index);
    io_outb(VGA_REGISTER_ARINDEX, value);

}

static void outcrt1(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_CRTINDEX1, (value << 8) | index);

}

static void outgr(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_GRINDEX, (value << 8) | index);

}

static void outsr(unsigned char index, unsigned char value)
{

    io_outw(VGA_REGISTER_SRINDEX, (value << 8) | index);

}

static void set_regs(const unsigned char *regs)
{

    io_outb(VGA_REGISTER_MISCWRITE, VGA_MISC_COLOR | VGA_MISC_ENABLE | VGA_MISC_PAGESELECT | VGA_MISC_SYNC400);

    outcrt1(VGA_CRTINDEX_CRT00, 0x5F);
    outcrt1(VGA_CRTINDEX_CRT01, 0x4F);
    outcrt1(VGA_CRTINDEX_CRT02, 0x50);
    outcrt1(VGA_CRTINDEX_CRT03, 0x82);
    outcrt1(VGA_CRTINDEX_CRT04, 0x54);
    outcrt1(VGA_CRTINDEX_CRT05, 0x80);
    outcrt1(VGA_CRTINDEX_CRT06, 0xBF);
    outcrt1(VGA_CRTINDEX_CRT07, 0x1F);
    outcrt1(VGA_CRTINDEX_CRT08, 0x00);
    outcrt1(VGA_CRTINDEX_CRT09, 0x41);
    outcrt1(VGA_CRTINDEX_CRT10, 0x9C);
    outcrt1(VGA_CRTINDEX_CRT11, VGA_CRT11_PROTECT | 0x0E);
    outcrt1(VGA_CRTINDEX_CRT12, 0x8F);
    outcrt1(VGA_CRTINDEX_CRT13, 0x28);
    outcrt1(VGA_CRTINDEX_CRT14, 0x40);
    outcrt1(VGA_CRTINDEX_CRT15, 0x96);
    outcrt1(VGA_CRTINDEX_CRT16, 0x89);
    outcrt1(VGA_CRTINDEX_CRT17, VGA_CRT17_MAP13 | VGA_CRT17_MAP14 | VGA_CRT17_AW | VGA_CRT17_SE);

    outsr(VGA_SRINDEX_SR01, VGA_SR01_DIV8);
    outsr(VGA_SRINDEX_SR02, VGA_SR02_ENABLE0 | VGA_SR02_ENABLE1 | VGA_SR02_ENABLE2 | VGA_SR02_ENABLE3);
    outsr(VGA_SRINDEX_SR03, VGA_SR03_P0K);
    outsr(VGA_SRINDEX_SR04, VGA_SR04_EXTENDED | VGA_SR04_ODDEVEN | VGA_SR04_CHAIN);

    outgr(VGA_GRINDEX_GR05, VGA_GR05_256MODE);
    outgr(VGA_GRINDEX_GR06, VGA_GR06_GRAPHICS | VGA_GR06_EGAVGA);

    outar(VGA_ARINDEX_AR10, VGA_AR10_GRAPHICS | VGA_AR10_PIXCLK);
    outar(VGA_ARINDEX_AR11, 0x00);
    outar(VGA_ARINDEX_AR12, VGA_AR12_ENABLE0 | VGA_AR12_ENABLE1 | VGA_AR12_ENABLE2 | VGA_AR12_ENABLE3);
    outar(VGA_ARINDEX_AR13, 0x00);
    outar(VGA_ARINDEX_AR14, 0x00);

    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_ENABLE);

}

static void mode(struct base_device *device)
{

    set_regs(g320x200x256);

}

static unsigned int read_terminal_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write_terminal_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)device->driver;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            driver->cursor.offset--;

        if (c == '\t')
            driver->cursor.offset = (driver->cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            driver->cursor.offset -= (driver->cursor.offset % 80);

        if (c == '\n')
            driver->cursor.offset += 80 - (driver->cursor.offset % 80);

        if (c >= ' ')
        {

            ((char *)VGA_TEXT)[driver->cursor.offset * 2] = c;
            ((char *)VGA_TEXT)[driver->cursor.offset * 2 + 1] = driver->cursor.color;

            driver->cursor.offset++;

        }

        if (driver->cursor.offset >= 2000)
        {

            unsigned int a;

            memory_copy((void *)VGA_TEXT, (void *)(VGA_TEXT + 80 * 2), 80 * 24 * 2);

            for (a = 80 * 24 * 2; a < 80 * 25 * 2; a += 2)
            {

                ((char *)VGA_TEXT)[a] = ' ';
                ((char *)VGA_TEXT)[a + 1] = driver->cursor.color;

            }

            driver->cursor.offset -= 80;

        }

        outcrt1(VGA_CRTINDEX_CRT0E, driver->cursor.offset >> 8);
        outcrt1(VGA_CRTINDEX_CRT0F, driver->cursor.offset);

    }

    return count;

}

static unsigned int read_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)device->driver;

    return memory_read(buffer, count, (void *)VGA_ADDRESS, driver->ivideo.xres * driver->ivideo.yres * (driver->ivideo.bpp / 8), offset);

}

static unsigned int write_video_data(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_driver *driver = (struct vga_driver *)device->driver;

    return memory_write((void *)VGA_ADDRESS, driver->ivideo.xres * driver->ivideo.yres * (driver->ivideo.bpp / 8), buffer, count, offset);

}

static unsigned int read_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > 256)
        count = 256;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACRINDEX, i / 3);
        c[i + 0] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 1] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 2] = io_inb(VGA_REGISTER_DACDATA);

    }

    return i - offset;

}

static unsigned int write_video_colormap(struct base_device *device, unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > 256)
        count = 256;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACWINDEX, i / 3);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return i - offset;

}

static void attach(struct base_device *device)
{

    struct vga_driver *driver = (struct vga_driver *)device->driver;
    unsigned int a;

    base_terminal_register_interface(&driver->iterminal, device);
    base_video_register_interface(&driver->ivideo, device);

    driver->cursor.color = 0x0F;

    for (a = 0; a < 80 * 25 * 2; a += 2)
    {

        ((char *)VGA_TEXT)[a] = ' ';
        ((char *)VGA_TEXT)[a + 1] = driver->cursor.color;

    }

}

static unsigned int check(struct base_device *device)
{

    struct pci_device *pciDevice = (struct pci_device *)device;

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    return pci_device_inb(pciDevice, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_device_inb(pciDevice, PCI_CONFIG_SUBCLASS) == 0x00 && pci_device_inb(pciDevice, PCI_CONFIG_INTERFACE) == 0x00;

}

void vga_init_driver(struct vga_driver *driver)
{

    memory_clear(driver, sizeof (struct vga_driver));
    base_init_driver(&driver->base, "vga", check, attach);
    base_terminal_init_interface(&driver->iterminal, read_terminal_data, write_terminal_data);
    base_video_init_interface(&driver->ivideo, mode, read_video_data, write_video_data, read_video_colormap, write_video_colormap);

    driver->ivideo.xres = 80;
    driver->ivideo.yres = 25;
    driver->ivideo.bpp = 16;

}

