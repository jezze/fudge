struct i915_driver
{

    struct base_driver base;
    struct base_video_interface ivideo;

};

void i915_init_driver(struct i915_driver *driver);
