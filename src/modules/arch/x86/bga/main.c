#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
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
static unsigned int framebuffer;

static void setreg(unsigned short index, unsigned short data)
{

    io_outw(REGISTERCOMMAND, index);
    io_outw(REGISTERDATA, data);

}

static unsigned int videointerface_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, &videointerface.settings, sizeof (struct ctrl_videosettings), offset);

}

static unsigned int videointerface_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_videosettings *settings = buffer;

    ctrl_setvideosettings(&videointerface.settings, settings->w, settings->h, settings->bpp);
    setreg(COMMANDENABLE, 0x00);
    setreg(COMMANDXRES, videointerface.settings.w);
    setreg(COMMANDYRES, videointerface.settings.h);
    setreg(COMMANDBPP, videointerface.settings.bpp * 8);
    setreg(COMMANDENABLE, 0x40 | 0x01);
    video_notifymode(&videointerface, framebuffer, videointerface.settings.w, videointerface.settings.h, videointerface.settings.bpp);

    return count;

}

static unsigned int videointerface_readdata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, (void *)framebuffer, videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp, offset);

}

static unsigned int videointerface_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write((void *)framebuffer, videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp, buffer, count, offset);

}

static unsigned int videointerface_readcolormap(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int videointerface_writecolormap(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return count;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id);

    videointerface.ctrl.operations.read = videointerface_readctrl;
    videointerface.ctrl.operations.write = videointerface_writectrl;
    videointerface.data.operations.read = videointerface_readdata;
    videointerface.data.operations.write = videointerface_writedata;
    videointerface.colormap.operations.read = videointerface_readcolormap;
    videointerface.colormap.operations.write = videointerface_writecolormap;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == PCIVENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == PCIDEVICE;

}

static void driver_reset(unsigned int id)
{

    framebuffer = pci_ind(id, PCI_CONFIG_BAR0) & 0xFFFFFFF0;

    arch_setmapshared(4, framebuffer, framebuffer, 0x00400000);
    arch_setmapshared(5, framebuffer + 0x00400000, framebuffer + 0x00400000, 0x00400000);

}

static void driver_attach(unsigned int id)
{

    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "bga", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}

