#include <modules.h>
#include <arch/x86/acpi/acpi.h>

static struct acpi_driver driver;

struct acpi_sdth *acpi_find_header(char *name)
{

    return driver.find_header(&driver, name);

}

void init()
{

    acpi_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

