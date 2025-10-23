#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/base/driver.h>
#include <modules/video/video.h>
#include <modules/arch/x86/io/io.h>
#include <modules/arch/x86/pci/pci.h>

#define VENDOR                          0x1234
#define DEVICE                          0x1111
#define REG_COMMAND                     0x01CE
#define REG_COMMAND_ID                  0x0000
#define REG_COMMAND_XRES                0x0001
#define REG_COMMAND_YRES                0x0002
#define REG_COMMAND_BPP                 0x0003
#define REG_COMMAND_ENABLE              0x0004
#define REG_COMMAND_BANK                0x0005
#define REG_COMMAND_VIRTWIDTH           0x0006
#define REG_COMMAND_VIRTHEIGHT          0x0007
#define REG_COMMAND_XOFF                0x0008
#define REG_COMMAND_YOFF                0x0009
#define REG_DATA                        0x01CF

static struct base_driver driver;
static struct video_interface videointerface;
static unsigned int framebuffer;

static void setreg(unsigned short index, unsigned short data)
{

    io_outw(REG_COMMAND, index);
    io_outw(REG_DATA, data);

}

static unsigned int videointerface_onvideocmap(unsigned int source, unsigned int count, void *buffer)
{

    return MESSAGE_UNIMPLEMENTED;

}

static unsigned int videointerface_onvideoconf(unsigned int source, unsigned int width, unsigned int height, unsigned int bpp)
{

    videointerface.width = width;
    videointerface.height = height;
    videointerface.bpp = bpp;

    setreg(REG_COMMAND_ENABLE, 0x00);
    setreg(REG_COMMAND_XRES, videointerface.width);
    setreg(REG_COMMAND_YRES, videointerface.height);
    setreg(REG_COMMAND_BPP, videointerface.bpp * 8);
    setreg(REG_COMMAND_ENABLE, 0x40 | 0x01);
    arch_umapvideo(framebuffer, framebuffer, 0x00400000);
    arch_umapvideo(framebuffer + 0x00400000, framebuffer + 0x00400000, 0x00400000);

    video_notifymode(&videointerface, (void *)framebuffer, videointerface.width, videointerface.height, videointerface.bpp);

    return MESSAGE_OK;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, videointerface_onvideocmap, videointerface_onvideoconf);

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR && pci_inw(id, PCI_CONFIG_DEVICE) == DEVICE;

}

static void driver_reset(unsigned int id)
{

    framebuffer = pci_ind(id, PCI_CONFIG_BAR0) & 0xFFFFFFF0;

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

