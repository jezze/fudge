#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define PCIVENDOR                       0x1234
#define PCIDEVICE                       0x1111
#define REGISTERCOMMAND                 0x01CE
#define REGISTERDATA                    0x01CF
#define COMMANDID                       0x0000
#define COMMANDXRES                     0x0001
#define COMMANDYRES                     0x0002
#define COMMANDBPP                      0x0003
#define COMMANDENABLE                   0x0004
#define COMMANDBANK                     0x0005
#define COMMANDVIRTWIDTH                0x0006
#define COMMANDVIRTHEIGHT               0x0007
#define COMMANDXOFF                     0x0008
#define COMMANDYOFF                     0x0009
#define BPP4                            0x04
#define BPP8                            0x08
#define BPP15                           0x0F
#define BPP16                           0x10
#define BPP24                           0x18
#define BPP32                           0x20

static struct base_driver driver;
static struct video_interface videointerface;
static void *bank;
static void *lfb;

static void setreg(unsigned short index, unsigned short data)
{

    io_outw(REGISTERCOMMAND, index);
    io_outw(REGISTERDATA, data);

}

static void videointerface_setmode(struct ctrl_videosettings *settings)
{

    ctrl_setvideosettings(&videointerface.settings, settings->w, settings->h, settings->bpp);
    setreg(COMMANDENABLE, 0x00);
    setreg(COMMANDXRES, videointerface.settings.w);
    setreg(COMMANDYRES, videointerface.settings.h);
    setreg(COMMANDBPP, videointerface.settings.bpp);
    setreg(COMMANDENABLE, 0x40 | 0x01);
    video_notifymode(&videointerface, videointerface.settings.w, videointerface.settings.h, videointerface.settings.bpp);

}

static unsigned int videointerface_rdata(unsigned int offset, void *buffer, unsigned int count)
{

    unsigned int s = videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp / 8;

    return memory_read(buffer, count, lfb, s, offset);

}

static unsigned int videointerface_wdata(unsigned int offset, void *buffer, unsigned int count)
{

    unsigned int *g = lfb;
    unsigned int *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        g[i + offset] = b[i];

    return count;

}

static unsigned int videointerface_rcolormap(unsigned int offset, void *buffer, unsigned int count)
{

    return 0;

}

static unsigned int videointerface_wcolormap(unsigned int offset, void *buffer, unsigned int count)
{

    return 0;

}

static void driver_init(void)
{

    video_initinterface(&videointerface, videointerface_setmode, videointerface_rdata, videointerface_wdata, videointerface_rcolormap, videointerface_wcolormap);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == PCIDEVICE;

}

static void driver_attach(unsigned int id)
{

    bank = (void *)0xA0000;
    lfb = (void *)(unsigned int)(pci_ind(id, PCI_CONFIG_BAR0) & 0xFFFFFFF0);

    arch_setmap(4, (unsigned int)lfb, (unsigned int)lfb, 0x00400000);
    arch_setmap(5, (unsigned int)lfb + 0x00400000, (unsigned int)lfb + 0x00400000, 0x00400000);
    video_registerinterface(&videointerface, id);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "bga", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

