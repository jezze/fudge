#include <module.h>
#include <base/base.h>
#include <base/video.h>
#include "i915.h"
#include "driver.h"

void init()
{

    i915_driver_init();

}

void destroy()
{

    i915_driver_destroy();

}

