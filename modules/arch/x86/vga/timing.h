#define PHSYNC                          0x0001
#define NHSYNC                          0x0002
#define PVSYNC                          0x0004
#define NVSYNC                          0x0008
#define INTERLACED                      0x0010
#define DOUBLESCAN                      0x0020
#define HADJUSTED                       0x0040
#define VADJUSTED                       0x0080
#define USEPROGRCLOCK                   0x0100
#define TVMODE                          0x0200
#define TVPAL                           0x0400
#define TVNTSC                          0x0800
#define CLOCK_PROGRAMMABLE              0x0001
#define INTERLACE_DIVIDE_VERT           0x0002
#define GREATER_1024_DIVIDE_VERT        0x0004
#define NO_RGB16_565                    0x0008
#define NO_INTERLACE                    0x0010
#define EMULATE_BANK                    0x0020
#define VGA_BPP2                        2
#define VGA_BPP4                        4
#define VGA_BPP8                        8
#define VGA_BPP15                       15
#define VGA_BPP16                       16
#define VGA_BPP24                       24
#define VGA_BPP32                       32
#define VGA_COLOR1                      (1 << 1)
#define VGA_COLOR4                      (1 << 4)
#define VGA_COLOR8                      (1 << 8)
#define VGA_COLOR15                     (1 << 15)
#define VGA_COLOR16                     (1 << 16)
#define VGA_COLOR24                     (1 << 24)

struct vga_monitormodetiming
{

    unsigned int pixelClock;
    unsigned int HDisplay;
    unsigned int HSyncStart;
    unsigned int HSyncEnd;
    unsigned int HTotal;
    unsigned int VDisplay;
    unsigned int VSyncStart;
    unsigned int VSyncEnd;
    unsigned int VTotal;
    unsigned int flags;
    struct vga_monitormodetiming *next;

};

struct vga_modetiming
{

    unsigned int pixelClock;
    unsigned int HDisplay;
    unsigned int HSyncStart;
    unsigned int HSyncEnd;
    unsigned int HTotal;
    unsigned int VDisplay;
    unsigned int VSyncStart;
    unsigned int VSyncEnd;
    unsigned int VTotal;
    unsigned int flags;
    unsigned int programmedClock;
    unsigned int selectedClockNo;
    unsigned int CrtcHDisplay;
    unsigned int CrtcHSyncStart;
    unsigned int CrtcHSyncEnd;
    unsigned int CrtcHTotal;
    unsigned int CrtcVDisplay;
    unsigned int CrtcVSyncStart;
    unsigned int CrtcVSyncEnd;
    unsigned int CrtcVTotal;

};

struct vga_cardspecs
{

    unsigned int videoMemory;
    unsigned int maxPixelClock4bpp;
    unsigned int maxPixelClock8bpp;
    unsigned int maxPixelClock16bpp;
    unsigned int maxPixelClock24bpp;
    unsigned int maxPixelClock32bpp;
    unsigned int flags;
    unsigned int nClocks;
    int *clocks;
    int maxHorizontalCrtc;
    int (*mapClock) (int bpp, int pixelclock);
    int (*matchProgrammableClock) (int desiredclock);
    int (*mapHorizontalCrtc) (int bpp, int pixelclock, int htiming);

};

struct vga_modeinfo
{

    unsigned short width;
    unsigned short height;
    unsigned char bytespp;
    unsigned char bpp;
    unsigned char colorBits;
    unsigned char redWeight;
    unsigned char greenWeight;
    unsigned char blueWeight;
    unsigned char redOffset;
    unsigned char blueOffset;
    unsigned char greenOffset;
    unsigned int redMask;
    unsigned int blueMask;
    unsigned int greenMask;
    unsigned int lineWidth;
    unsigned short realWidth;
    unsigned short realHeight;
    unsigned int flags;

};

struct vga_info
{

    int xdim;
    int ydim;
    int colors;
    int xbytes;
    int bytespp;

};

void vga_save();
void vga_restore();
void vga_settext();
void vga_setregisters(unsigned char *registers);
void vga_initregisters(unsigned char *registers, struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo);
void vga_initmodetiming(struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo, struct vga_cardspecs *cardspecs);
void vga_initmodeinfo(struct vga_modeinfo *modeinfo, int mode);
