#ifndef MODULES_I915_H
#define MODULES_I915_H

#define I915_DRIVER_TYPE 0x0915

struct i915_driver
{

    struct modules_driver base;

};

extern void i915_driver_init(struct i915_driver *driver);

#endif

