#define MBOOT_FLAG_MEMORY               0x00000001
#define MBOOT_FLAG_DEVICE               0x00000002
#define MBOOT_FLAG_CMDLINE              0x00000004
#define MBOOT_FLAG_MODULES              0x00000008
#define MBOOT_FLAG_AOUT                 0x00000010
#define MBOOT_FLAG_ELF                  0x00000020
#define MBOOT_FLAG_MMAP                 0x00000040
#define MBOOT_FLAG_CONFIG               0x00000080
#define MBOOT_FLAG_LOADER               0x00000100
#define MBOOT_FLAG_APM                  0x00000200
#define MBOOT_FLAG_VBE                  0x00000400
#define MBOOT_FLAG_FRAMEBUFFER          0x00000800

struct mboot_header_modules
{

    unsigned int count;
    unsigned int address;

};

struct mboot_header_elf
{

    unsigned int shcount;
    unsigned int shsize;
    unsigned int shaddress;
    unsigned int shstringindex;

};

struct mboot_header_mmap
{

    unsigned int count;
    unsigned int address;

};

struct mboot_header_drives
{

    unsigned int count;
    unsigned int address;

};

struct mboot_header_vbe
{

    unsigned int cinfo;
    unsigned int minfo;
    unsigned short mode;
    unsigned short isegment;
    unsigned short ioffset;
    unsigned short ilength;

};

struct mboot_header_framebuffer
{

    unsigned int address[2];
    unsigned int width;
    unsigned int height;
    unsigned char bpp;
    unsigned char type;
    unsigned char color[6];

};

struct mboot_header
{

    unsigned int flags;
    unsigned int lomem;
    unsigned int himem;
    unsigned int device;
    unsigned int cmdline;
    struct mboot_header_modules modules;
    struct mboot_header_elf elf;
    struct mboot_header_mmap mmap;
    struct mboot_header_drives drives;
    unsigned int ctable;
    unsigned int name;
    unsigned int atable;
    struct mboot_header_vbe vbe;
    struct mboot_header_framebuffer framebuffer;

};

struct mboot_mmap
{

    unsigned int size;
    unsigned int address0;
    unsigned int address1;
    unsigned int limit0;
    unsigned int limit1;
    unsigned int type;

};

struct mboot_module
{

    unsigned int address;
    unsigned int limit;
    unsigned int name;
    unsigned int reserved;

};

void mboot_setup(struct mboot_header *header, unsigned int magic);
