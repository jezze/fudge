#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/video.h>
#include <modules/icp/icp.h>

#define MAXWIDTH                        1024
#define MAXHEIGHT                       1024
#define BPP                             4
#define REGISTERS                       0xB0000000
#define REG_TIMING0                     0x00
#define REG_TIMING1                     0x04
#define REG_TIMING2                     0x08
#define REG_UPBASE                      0x10
#define REG_CONTROL                     0x1C
#define REG_CONTROL_ENABLE              (1 << 0)
#define REG_CONTROL_BPP24               (5 << 1)
#define REG_CONTROL_TFT                 (1 << 5)
#define REG_CONTROL_BGR                 (1 << 8)
#define REG_CONTROL_POWER               (1 << 11)

static struct base_driver driver;
static struct video_interface videointerface;
static volatile unsigned int *regs = (volatile unsigned int *)REGISTERS;

/* a mode larger than the controller's keeps its aspect ratio; pixels per line count in 16s and only 32-bit pixels are offered */
static void videointerface_setmode(unsigned int width, unsigned int height, unsigned int bpp)
{

    if (width > MAXWIDTH)
    {

        height = height * MAXWIDTH / width;
        width = MAXWIDTH;

    }

    if (height > MAXHEIGHT)
    {

        width = width * MAXHEIGHT / height;
        height = MAXHEIGHT;

    }

    width &= ~15;

    if (width * height * BPP > ARCH_FRAMEBUFFER_SIZE)
        height = ARCH_FRAMEBUFFER_SIZE / (width * BPP);

    videointerface.width = width;
    videointerface.height = height;
    videointerface.bpp = BPP;

    arch_kmap(ARCH_FRAMEBUFFER_BASE, 0xA0000000, width * height * BPP, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE | MMAP_FLAG_WRITETHROUGH);

    regs[REG_CONTROL / 4] = 0;
    regs[REG_TIMING0 / 4] = ((width / 16) - 1) << 2;
    regs[REG_TIMING1 / 4] = height - 1;
    regs[REG_TIMING2 / 4] = (width - 1) << 16;
    regs[REG_UPBASE / 4] = ARCH_FRAMEBUFFER_BASE;
    regs[REG_CONTROL / 4] = REG_CONTROL_ENABLE | REG_CONTROL_BPP24 | REG_CONTROL_TFT | REG_CONTROL_BGR | REG_CONTROL_POWER;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id, 0xA0000000, 0, videointerface_setmode);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_LCD;

}

/* the registers are at 0xC0000000, where each task's mmap is mapped, so they get another address */
static void driver_reset(unsigned int id)
{

    arch_kmap(icp_getbase(id), REGISTERS, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

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

    base_initdriver(&driver, "pl110", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}
