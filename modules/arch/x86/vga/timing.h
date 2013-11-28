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

enum vga_cardspecs_flags
{

    CLOCK_PROGRAMMABLE                  = 0x0001,
    INTERLACE_DIVIDE_VERT               = 0x0002,
    GREATER_1024_DIVIDE_VERT            = 0x0004,
    NO_RGB16_565                        = 0x0008,
    NO_INTERLACE                        = 0x0010,
    EMULATE_BANK                        = 0x0020

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

enum
{

    VGA_BPP2                            = 2,
    VGA_BPP4                            = 4,
    VGA_BPP8                            = 8,
    VGA_BPP15                           = 15,
    VGA_BPP16                           = 16,
    VGA_BPP24                           = 24,
    VGA_BPP32                           = 32

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

enum vga_info_colors
{

    VGA_COLOR1                          = (1 << 1),
    VGA_COLOR4                          = (1 << 4),
    VGA_COLOR8                          = (1 << 8),
    VGA_COLOR15                         = (1 << 15),
    VGA_COLOR16                         = (1 << 16),
    VGA_COLOR24                         = (1 << 24)

};

struct vga_info
{

    int xdim;
    int ydim;
    int colors;
    int xbytes;
    int bytespp;

};

void vga_setregisters(unsigned char *registers, unsigned int ega);
void vga_initregisters(unsigned char *registers, struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo);
void vga_initmodetiming(struct vga_modetiming *modetiming, struct vga_modeinfo *modeinfo, struct vga_cardspecs *cardspecs);
void vga_initmodeinfo(struct vga_modeinfo *modeinfo, int mode);
