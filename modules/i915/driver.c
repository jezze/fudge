#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/i915/i915.h>

static void wait_vblank()
{

    unsigned volatile int *stat = (unsigned volatile int *)I915_PIPEA_STAT;

    *stat = *stat | I915_PIPE_STAT_STATUS_VBLANK;

    while (*stat & I915_PIPE_STAT_STATUS_VBLANK);

}

static void enable_dpll()
{

    unsigned volatile int *ctrl = (unsigned volatile int *)I915_DPLLA_CTRL;

    //unsigned int temp = I915_DPLL_CTRL_DISABLE_VGA | I915_DPLL_CTRL_DIVIDE2;

    *ctrl = I915_DPLL_CTRL_ENABLE_VCO;

}

static void enable_pipe()
{

    unsigned volatile int *conf = (unsigned volatile int *)I915_PIPEA_CONF;

    *conf = *conf & I915_PIPE_CONF_ENABLE;

    wait_vblank();

}

static void enable_plane()
{

    unsigned volatile int *crtl = (unsigned volatile int *)I915_DISPLAYA_CTRL;

    *crtl = *crtl & I915_DISPLAY_CTRL_ENABLE_PLANE;

    unsigned volatile int *addr = (unsigned volatile int *)I915_DISPLAYA_ADDR;

    *addr = *addr;

    unsigned volatile int *surf = (unsigned volatile int *)I915_DISPLAYA_SURF;

    *surf = *surf;

    wait_vblank();

}

static void set_pipe_mode(unsigned int width, unsigned int height)
{

    unsigned volatile int *htotal = (unsigned volatile int *)I915_DISPLAYA_HTOTAL;

    *htotal = ((height - 1) << 16) | ((height - 1) << 0);

    unsigned volatile int *hblank = (unsigned volatile int *)I915_DISPLAYA_HBLANK;

    *hblank = ((height - 1) << 16) | ((height - 1) << 0);

    unsigned volatile int *hsync = (unsigned volatile int *)I915_DISPLAYA_HSYNC;

    *hsync = ((height - 1) << 16) | ((height - 1) << 0);

    unsigned volatile int *vtotal = (unsigned volatile int *)I915_DISPLAYA_VTOTAL;

    *vtotal = ((width - 1) << 16) | ((width - 1) << 0);

    unsigned volatile int *vblank = (unsigned volatile int *)I915_DISPLAYA_VBLANK;

    *vblank = ((width - 1) << 16) | ((width - 1) << 0);

    unsigned volatile int *vsync = (unsigned volatile int *)I915_DISPLAYA_VSYNC;

    *vsync = ((width - 1) << 16) | ((width - 1) << 0);

}

static void start(struct modules_driver *self)
{

    unsigned volatile int *lvds = (unsigned volatile int *)0x61180;

    log_write("[i915] LVDS: %x\n", *lvds);

    log_write("[i915] Set pipe mode\n");
    set_pipe_mode(1024, 600);

    log_write("[i915] SKIP: Enable panelfitter\n");

    log_write("[i915] Enable dpll\n");
    enable_dpll();

    log_write("[i915] SKIP: Wait 150us\n");

    log_write("[i915] Enable pipe\n");
    enable_pipe();

    log_write("[i915] Enable plane\n");
    enable_plane();

    log_write("[i915] SKIP: Enable ports\n");

}

void handle_irq()
{

    log_write("[i915] IRQ\n");

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

    struct i915_driver *driver = (struct i915_driver *)self;
    struct pci_device *pciDevice = (struct pci_device *)device;

    irq_register_routine(pciDevice->configuration.interruptline, device, handle_irq);

}

static unsigned int check(struct modules_driver *self, struct modules_device *device)
{

    if (device->type != PCI_DEVICE_TYPE)
        return 0;

    struct pci_device *pciDevice = (struct pci_device *)device;

    return pciDevice->configuration.vendorid == 0x8086 && pciDevice->configuration.deviceid == 0x27AE;

}

void i915_driver_init(struct i915_driver *driver)
{

    modules_driver_init(&driver->base, I915_DRIVER_TYPE);

    driver->base.start = start;
    driver->base.attach = attach;
    driver->base.check = check;

}

