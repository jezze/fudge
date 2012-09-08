#include <base/base.h>
#include <arch/x86/i915/i915.h>

static struct i915_driver driver;

void init()
{

    i915_driver_init(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);

}

