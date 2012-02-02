#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/i915/i915.h>

#define I915_PIPEA_DSL  0x70000
#define I915_PIPEA_SCL  0x70004
#define I915_PIPEA_CONF 0x70008
#define I915_PIPEA_STAT 0x70024

#define I915_PIPEB_DSL  0x71000
#define I915_PIPEB_SCL  0x71004
#define I915_PIPEB_CONF 0x71008
#define I915_PIPEB_STAT 0x71024

#define I915_PIPE_CONF_STATE  (1 << 30)
#define I915_PIPE_CONF_ENABLE (1 << 31)

#define I915_PIPE_STAT_STATUS_VBLANK (1 << 1)
#define I915_PIPE_STAT_STATUS_VSYNC (1 << 9)

#define I915_DISPLAYA_CNTR 0x70180
#define I915_DISPLAYA_LOFF 0x70184
#define I915_DISPLAYA_SURF 0x7019C

#define I915_DISPLAYB_CNTR 0x71180
#define I915_DISPLAYB_LOFF 0x71184
#define I915_DISPLAYB_SURF 0x7119C

#define I915_DISPLAY_CNTR_ENABLE_GAMMA (1 << 30)
#define I915_DISPLAY_CNTR_ENABLE_PLANE (1 << 31)

static void wait_vblank()
{

    unsigned volatile int *stat = (unsigned volatile int *)I915_PIPEA_STAT;

    *stat = *stat | I915_PIPE_STAT_STATUS_VBLANK;

    while (*stat & I915_PIPE_STAT_STATUS_VBLANK);

}

static void enable_pipe()
{

    unsigned volatile int *conf = (unsigned volatile int *)I915_PIPEA_CONF;

    *conf = *conf & I915_PIPE_CONF_ENABLE;

    wait_vblank();

}

static void enable_plane()
{

    unsigned volatile int *cntr = (unsigned volatile int *)I915_DISPLAYA_CNTR;

    *cntr = *cntr & I915_DISPLAY_CNTR_ENABLE_PLANE;

    // flush plane

    wait_vblank();

}

static void i915_driver_start(struct modules_driver *self)
{

    // Program pipe timings

    // Enable panelfitter as needed



    // Program DPLL

    // Enable DPLL

    // Wait 150us

    enable_pipe();
    enable_plane();

    // Enable ports

}

void i915_driver_init(struct i915_driver *driver)
{

    modules_driver_init(&driver->base, I915_DRIVER_TYPE);

    driver->base.start = i915_driver_start;

}

