#include <lib/memory.h>
#include <kernel/irq.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/i915/i915.h>

static unsigned int read(unsigned int reg)
{

    volatile unsigned int *value = (volatile unsigned int *)reg;

    return *value;

}

static void write(unsigned int reg, unsigned int val)
{

    volatile unsigned int *value = (volatile unsigned int *)reg;

    *value = val;

}

static void wait(unsigned int num)
{

    unsigned int i = 0xcfffffff;
    unsigned int j = 0xcfffffff;

    while (i--);

}

static void wait_vblank()
{

    log_write("[i915] Wait vblank\n");

    write(I915_PIPEB_STATUS, I915_PIPE_STATUS_VBLANK);

    while (!(read(I915_PIPEB_STATUS) & I915_PIPE_STATUS_VBLANK));

}

static void enable_dpll()
{

    unsigned int pllb = read(I915_DPLLB_CONTROL);

    if (!(pllb & I915_DPLL_CONTROL_ENABLE_VCO))
    {

        log_write("[i915] Enable dpll\n");

        write(I915_DPLLB_CONTROL, pllb);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);
        write(I915_DPLLB_CONTROL, pllb | I915_DPLL_CONTROL_ENABLE_VCO);
        read(I915_DPLLB_CONTROL);
        wait(150);

    }

}

static void enable_pipe()
{

    log_write("[i915] Enable pipe\n");

    write(I915_PIPEB_CONFIG, read(I915_PIPEB_CONFIG) & I915_PIPE_CONFIG_ENABLE);

}

static void enable_plane()
{

    log_write("[i915] Enable plane\n");

    write(I915_DISPLAYB_CONTROL, read(I915_DISPLAYB_CONTROL) & I915_DISPLAY_CONTROL_ENABLE_PLANE);

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

    log_write("[i915] PIPEACONF: 0x%x\n", read(I915_PIPEA_CONFIG));
    log_write("[i915] PIPEBCONF: 0x%x\n", read(I915_PIPEB_CONFIG));

    enable_dpll();
    enable_pipe();
    enable_plane();
    wait_vblank();

}

static void handle_irq()
{

    log_write("[i915] IRQ\n");

}

static void attach(struct modules_driver *self, struct modules_device *device)
{

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

    memory_clear(driver, sizeof (struct i915_driver));

    modules_driver_init(&driver->base, I915_DRIVER_TYPE, "i915");

    driver->base.start = start;
    driver->base.attach = attach;
    driver->base.check = check;

}

