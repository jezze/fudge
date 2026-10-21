#define SDL_LIL_ENDIAN                  1234
#define SDL_BIG_ENDIAN                  4321
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SDL_BYTEORDER                   SDL_BIG_ENDIAN
#define SDL_SwapLE16(x)                 ((Uint16)(((Uint16)(x) >> 8) | ((Uint16)(x) << 8)))
#define SDL_SwapLE32(x)                 ((Uint32)(((Uint32)(x) >> 24) | (((Uint32)(x) >> 8) & 0xFF00) | (((Uint32)(x) << 8) & 0xFF0000) | ((Uint32)(x) << 24)))
#define SDL_SwapBE16(x)                 ((Uint16)(x))
#define SDL_SwapBE32(x)                 ((Uint32)(x))
#else
#define SDL_BYTEORDER                   SDL_LIL_ENDIAN
#define SDL_SwapLE16(x)                 ((Uint16)(x))
#define SDL_SwapLE32(x)                 ((Uint32)(x))
#define SDL_SwapBE16(x)                 ((Uint16)(((Uint16)(x) >> 8) | ((Uint16)(x) << 8)))
#define SDL_SwapBE32(x)                 ((Uint32)(((Uint32)(x) >> 24) | (((Uint32)(x) >> 8) & 0xFF00) | (((Uint32)(x) << 8) & 0xFF0000) | ((Uint32)(x) << 24)))
#endif
#define SDL_COMPILE_TIME_ASSERT(name, x) typedef int SDL_compile_time_assert_ ## name[(x) * 2 - 1]
#define SDL_FALSE                       0
#define SDL_TRUE                        1
#define SDL_ALPHA_TRANSPARENT           0
#define SDL_ALPHA_OPAQUE                255
#define SDL_BLENDMODE_NONE              0
#define SDL_BLENDMODE_BLEND             1
#define SDL_PIXELFORMAT_UNKNOWN         0
#define SDL_PIXELFORMAT_INDEX8          1
#define SDL_PIXELFORMAT_RGB24           2
#define SDL_PIXELFORMAT_ARGB8888        3
#define SDL_PIXELFORMAT_ABGR8888        4
#define SDL_ISPIXELFORMAT_INDEXED(f)    ((f) == SDL_PIXELFORMAT_INDEX8)
#define SDL_KEYDOWN                     0x300
#define SDL_KEYUP                       0x301
#define SDL_QUIT                        0x100
#define KMOD_NONE                       0x0000
#define KMOD_LSHIFT                     0x0001
#define KMOD_RSHIFT                     0x0002
#define KMOD_LCTRL                      0x0040
#define KMOD_RCTRL                      0x0080
#define KMOD_LALT                       0x0100
#define KMOD_RALT                       0x0200
#define KMOD_SHIFT                      (KMOD_LSHIFT | KMOD_RSHIFT)
#define KMOD_CTRL                       (KMOD_LCTRL | KMOD_RCTRL)
#define KMOD_ALT                        (KMOD_LALT | KMOD_RALT)
#define SDL_SCANCODE_UNKNOWN            0
#define SDL_SCANCODE_A                  4
#define SDL_SCANCODE_B                  5
#define SDL_SCANCODE_C                  6
#define SDL_SCANCODE_D                  7
#define SDL_SCANCODE_E                  8
#define SDL_SCANCODE_F                  9
#define SDL_SCANCODE_G                  10
#define SDL_SCANCODE_H                  11
#define SDL_SCANCODE_I                  12
#define SDL_SCANCODE_J                  13
#define SDL_SCANCODE_K                  14
#define SDL_SCANCODE_L                  15
#define SDL_SCANCODE_M                  16
#define SDL_SCANCODE_N                  17
#define SDL_SCANCODE_O                  18
#define SDL_SCANCODE_P                  19
#define SDL_SCANCODE_Q                  20
#define SDL_SCANCODE_R                  21
#define SDL_SCANCODE_S                  22
#define SDL_SCANCODE_T                  23
#define SDL_SCANCODE_U                  24
#define SDL_SCANCODE_V                  25
#define SDL_SCANCODE_W                  26
#define SDL_SCANCODE_X                  27
#define SDL_SCANCODE_Y                  28
#define SDL_SCANCODE_Z                  29
#define SDL_SCANCODE_1                  30
#define SDL_SCANCODE_2                  31
#define SDL_SCANCODE_3                  32
#define SDL_SCANCODE_4                  33
#define SDL_SCANCODE_5                  34
#define SDL_SCANCODE_6                  35
#define SDL_SCANCODE_7                  36
#define SDL_SCANCODE_8                  37
#define SDL_SCANCODE_9                  38
#define SDL_SCANCODE_0                  39
#define SDL_SCANCODE_RETURN             40
#define SDL_SCANCODE_ESCAPE             41
#define SDL_SCANCODE_BACKSPACE          42
#define SDL_SCANCODE_TAB                43
#define SDL_SCANCODE_SPACE              44
#define SDL_SCANCODE_MINUS              45
#define SDL_SCANCODE_EQUALS             46
#define SDL_SCANCODE_LEFTBRACKET        47
#define SDL_SCANCODE_RIGHTBRACKET       48
#define SDL_SCANCODE_BACKSLASH          49
#define SDL_SCANCODE_SEMICOLON          51
#define SDL_SCANCODE_APOSTROPHE         52
#define SDL_SCANCODE_GRAVE              53
#define SDL_SCANCODE_COMMA              54
#define SDL_SCANCODE_PERIOD             55
#define SDL_SCANCODE_SLASH              56
#define SDL_SCANCODE_CAPSLOCK           57
#define SDL_SCANCODE_F1                 58
#define SDL_SCANCODE_F2                 59
#define SDL_SCANCODE_F3                 60
#define SDL_SCANCODE_F4                 61
#define SDL_SCANCODE_F5                 62
#define SDL_SCANCODE_F6                 63
#define SDL_SCANCODE_F7                 64
#define SDL_SCANCODE_F8                 65
#define SDL_SCANCODE_F9                 66
#define SDL_SCANCODE_F10                67
#define SDL_SCANCODE_F11                68
#define SDL_SCANCODE_F12                69
#define SDL_SCANCODE_PRINTSCREEN        70
#define SDL_SCANCODE_SCROLLLOCK         71
#define SDL_SCANCODE_PAUSE              72
#define SDL_SCANCODE_INSERT             73
#define SDL_SCANCODE_HOME               74
#define SDL_SCANCODE_PAGEUP             75
#define SDL_SCANCODE_DELETE             76
#define SDL_SCANCODE_END                77
#define SDL_SCANCODE_PAGEDOWN           78
#define SDL_SCANCODE_RIGHT              79
#define SDL_SCANCODE_LEFT               80
#define SDL_SCANCODE_DOWN               81
#define SDL_SCANCODE_UP                 82
#define SDL_SCANCODE_NUMLOCKCLEAR       83
#define SDL_SCANCODE_KP_DIVIDE          84
#define SDL_SCANCODE_KP_MULTIPLY        85
#define SDL_SCANCODE_KP_MINUS           86
#define SDL_SCANCODE_KP_PLUS            87
#define SDL_SCANCODE_KP_ENTER           88
#define SDL_SCANCODE_KP_1               89
#define SDL_SCANCODE_KP_2               90
#define SDL_SCANCODE_KP_3               91
#define SDL_SCANCODE_KP_4               92
#define SDL_SCANCODE_KP_5               93
#define SDL_SCANCODE_KP_6               94
#define SDL_SCANCODE_KP_7               95
#define SDL_SCANCODE_KP_8               96
#define SDL_SCANCODE_KP_9               97
#define SDL_SCANCODE_KP_0               98
#define SDL_SCANCODE_KP_PERIOD          99
#define SDL_SCANCODE_APPLICATION        101
#define SDL_SCANCODE_MUTE               127
#define SDL_SCANCODE_VOLUMEUP           128
#define SDL_SCANCODE_VOLUMEDOWN         129
#define SDL_SCANCODE_CLEAR              156
#define SDL_SCANCODE_LCTRL              224
#define SDL_SCANCODE_LSHIFT             225
#define SDL_SCANCODE_LALT               226
#define SDL_SCANCODE_LGUI               227
#define SDL_SCANCODE_RCTRL              228
#define SDL_SCANCODE_RSHIFT             229
#define SDL_SCANCODE_RALT               230
#define SDL_SCANCODE_RGUI               231
#define SDL_SCANCODE_AUDIOMUTE          262
#define SDL_NUM_SCANCODES               512

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned int Uint32;
typedef signed int Sint32;
typedef int SDL_Scancode;
typedef int SDL_TimerID;
typedef Uint32 (*SDL_TimerCallback)(Uint32 interval, void *param);
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Joystick SDL_Joystick;
typedef struct SDL_Haptic SDL_Haptic;
typedef struct SDL_GameController SDL_GameController;

typedef struct SDL_Color
{

    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

} SDL_Color;

typedef struct SDL_Rect
{

    int x;
    int y;
    int w;
    int h;

} SDL_Rect;

typedef struct SDL_Palette
{

    int ncolors;
    SDL_Color *colors;

} SDL_Palette;

typedef struct SDL_PixelFormat
{

    Uint32 format;
    SDL_Palette *palette;
    Uint8 BitsPerPixel;
    Uint8 BytesPerPixel;
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;

} SDL_PixelFormat;

typedef struct SDL_Surface
{

    SDL_PixelFormat *format;
    int w;
    int h;
    int pitch;
    void *pixels;
    SDL_Rect clip_rect;
    unsigned int hascolorkey;
    Uint32 colorkey;
    unsigned int blendmode;
    Uint8 alphamod;
    SDL_PixelFormat ownformat;
    SDL_Palette ownpalette;
    SDL_Color colors[256];

} SDL_Surface;

typedef struct SDL_Keysym
{

    SDL_Scancode scancode;
    Uint16 mod;

} SDL_Keysym;

typedef struct SDL_KeyboardEvent
{

    Uint32 type;
    SDL_Keysym keysym;

} SDL_KeyboardEvent;

typedef union SDL_Event
{

    Uint32 type;
    SDL_KeyboardEvent key;

} SDL_Event;

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 rmask, Uint32 gmask, Uint32 bmask, Uint32 amask);
SDL_Surface *SDL_ConvertSurface(SDL_Surface *surface, const SDL_PixelFormat *format, Uint32 flags);
SDL_Surface *SDL_ConvertSurfaceFormat(SDL_Surface *surface, Uint32 format, Uint32 flags);
void SDL_FreeSurface(SDL_Surface *surface);
int SDL_LockSurface(SDL_Surface *surface);
void SDL_UnlockSurface(SDL_Surface *surface);
int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);
int SDL_SetSurfaceBlendMode(SDL_Surface *surface, int mode);
int SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 alpha);
int SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette);
int SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors, int first, int count);
unsigned int SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect);
Uint32 SDL_MapRGB(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);
Uint32 SDL_MapRGBA(const SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int SDL_FillRect(SDL_Surface *surface, const SDL_Rect *rect, Uint32 color);
int SDL_BlitSurface(SDL_Surface *source, const SDL_Rect *sourcerect, SDL_Surface *target, SDL_Rect *targetrect);
SDL_Surface *SDL_LoadPNG(const void *data, unsigned int count);
int SDL_PollEvent(SDL_Event *event);
void SDL_Delay(Uint32 ms);
SDL_TimerID SDL_AddTimer(Uint32 interval, SDL_TimerCallback callback, void *param);
const Uint8 *SDL_GetKeyboardState(int *numkeys);
Uint32 SDL_GetTicks(void);
Uint32 SDL_GetPerformanceCounter(void);
Uint32 SDL_GetPerformanceFrequency(void);
void SDL_SetTextInputRect(const SDL_Rect *rect);
void SDL_StartTextInput(void);
void SDL_StopTextInput(void);
const char *SDL_GetError(void);
void SDL_Quit(void);
