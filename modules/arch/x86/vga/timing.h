#define CLOCK_PROGRAMMABLE              0x1
#define INTERLACE_DIVIDE_VERT           0x2
#define GREATER_1024_DIVIDE_VERT        0x4
#define NO_RGB16_565                    0x8
#define NO_INTERLACE                    0x10
#define EMULATE_BANK                    0x20

struct vga_monitormodetiming
{

    int pixelClock;
    int HDisplay;
    int HSyncStart;
    int HSyncEnd;
    int HTotal;
    int VDisplay;
    int VSyncStart;
    int VSyncEnd;
    int VTotal;
    int flags;
    struct vga_monitormodetiming *next;

};

enum vga_modetiming_flags
{

    PHSYNC                              = 0x0001,
    NHSYNC                              = 0x0002,
    PVSYNC                              = 0x0004,
    NVSYNC                              = 0x0008,
    INTERLACED                          = 0x0010,
    DOUBLESCAN                          = 0x0020,
    HADJUSTED                           = 0x0040,
    VADJUSTED                           = 0x0080,
    USEPROGRCLOCK                       = 0x0100,
    TVMODE                              = 0x0200,
    TVPAL                               = 0x0400,
    TVNTSC                              = 0x0800

};

struct vga_modetiming
{

    int pixelClock;
    int HDisplay;
    int HSyncStart;
    int HSyncEnd;
    int HTotal;
    int VDisplay;
    int VSyncStart;
    int VSyncEnd;
    int VTotal;
    int flags;
    int programmedClock;
    int selectedClockNo;
    int CrtcHDisplay;
    int CrtcHSyncStart;
    int CrtcHSyncEnd;
    int CrtcHTotal;
    int CrtcVDisplay;
    int CrtcVSyncStart;
    int CrtcVSyncEnd;
    int CrtcVTotal;

};

struct vga_cardspecs
{

    int videoMemory;
    int maxPixelClock4bpp;
    int maxPixelClock8bpp;
    int maxPixelClock16bpp;
    int maxPixelClock24bpp;
    int maxPixelClock32bpp;
    int flags;
    int nClocks;
    int *clocks;
    int maxHorizontalCrtc;
    int (*mapClock) (int bpp, int pixelclock);
    int (*matchProgrammableClock) (int desiredclock);
    int (*mapHorizontalCrtc) (int bpp, int pixelclock, int htiming);

};

struct vga_modeinfo
{

    short width;
    short height;
    char bytesPerPixel;
    char bitsPerPixel;
    char colorBits;
    char __padding1;
    char redWeight;
    char greenWeight;
    char blueWeight;
    char __padding2;
    char redOffset;
    char blueOffset;
    char greenOffset;
    char __padding3;
    unsigned redMask;
    unsigned blueMask;
    unsigned greenMask;
    int lineWidth;
    short realWidth;
    short realHeight;
    int flags;

};

struct vga_info
{

    int xdim;
    int ydim;
    int colors;
    int xbytes;
    int bytespp;

};

void vga_initregisters(unsigned char *registers, struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo);
void vga_initmodetiming(struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo, struct vga_cardspecs *cardspecs);
void vga_initmodeinfo(struct vga_modeinfo *modeinfo, int mode);
