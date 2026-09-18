#define MBOOT_TAG_END                   0
#define MBOOT_TAG_CMDLINE               1
#define MBOOT_TAG_BOOT_LOADER_NAME      2
#define MBOOT_TAG_MODULE                3
#define MBOOT_TAG_BASIC_MEMINFO         4
#define MBOOT_TAG_BOOTDEV               5
#define MBOOT_TAG_MMAP                  6
#define MBOOT_TAG_VBE                   7
#define MBOOT_TAG_FRAMEBUFFER           8
#define MBOOT_TAG_ELF_SECTIONS          9
#define MBOOT_TAG_APM                   10
#define MBOOT_TAG_EFI32                 11
#define MBOOT_TAG_EFI64                 12
#define MBOOT_TAG_SMBIOS                13
#define MBOOT_TAG_ACPI_OLD              14
#define MBOOT_TAG_ACPI_NEW              15
#define MBOOT_TAG_NETWORK               16
#define MBOOT_TAG_EFI_MMAP              17
#define MBOOT_TAG_EFI_BS                18
#define MBOOT_TAG_EFI32_IH              19
#define MBOOT_TAG_EFI64_IH              20

struct mboot_tag
{

    unsigned int type;
    unsigned int size;

};

struct mboot_tag_framebuffer
{

    unsigned int address[2];
    unsigned int pitch;
    unsigned int width;
    unsigned int height;
    unsigned char bpp;
    unsigned char type;
    unsigned short reserved;

};

struct mboot_tag_module
{

    unsigned int start;
    unsigned int end;

};

void mboot_setup(unsigned long address, unsigned long magic);
