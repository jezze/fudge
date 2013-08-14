#define MBOOT_MAGIC                     0x2BADB002

#define MBOOT_FLAG_MEMORY               (1 << 0)
#define MBOOT_FLAG_DEVICE               (1 << 1)
#define MBOOT_FLAG_CMDLINE              (1 << 2)
#define MBOOT_FLAG_MODULES              (1 << 3)
#define MBOOT_FLAG_AOUT                 (1 << 4)
#define MBOOT_FLAG_ELF                  (1 << 5)
#define MBOOT_FLAG_MMAP                 (1 << 6)
#define MBOOT_FLAG_CONFIG               (1 << 7)
#define MBOOT_FLAG_LOADER               (1 << 8)
#define MBOOT_FLAG_APM                  (1 << 9)
#define MBOOT_FLAG_VBE                  (1 << 10)

struct mboot_header_modules
{

    unsigned int count;
    void *address;

};

struct mboot_header_elf
{

    unsigned int shcount;
    unsigned int shsize;
    void *shaddress;
    unsigned int shstringindex;

};

struct mboot_header_mmap
{

    unsigned int count;
    void *address;

};

struct mboot_header_drives
{

    unsigned int count;
    void *address;

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
