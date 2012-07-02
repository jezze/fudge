#include <modules.h>
#include <acpi/acpi.h>

static struct acpi_driver driver;

void init()
{

    acpi_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

