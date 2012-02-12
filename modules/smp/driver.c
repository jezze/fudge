#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/acpi/acpi.h>
#include <modules/cpuid/cpuid.h>
#include <modules/smp/smp.h>

static void start(struct modules_driver *self)
{

    struct smp_driver *driver = (struct smp_driver *)self;

    struct acpi_driver *acpiDriver = (struct acpi_driver *)modules_get_driver(ACPI_DRIVER_TYPE);

    if (!acpiDriver)
        return;

    struct cpuid_driver *cpuidDriver = (struct cpuid_driver *)modules_get_driver(CPUID_DRIVER_TYPE);

    if (!cpuidDriver)
        return;

    log_write("[smp] Driver started\n");

    if (!cpuidDriver->is_supported(CPUID_INSTRUCTION_VENDOR, CPUID_FEATURES0_EDX_FLAG_HTT))
        return;

    struct acpi_madt *madt = (struct acpi_madt *)acpiDriver->find_header(acpiDriver, "APIC");

    log_write("[smp] Madt signature: %c%c%c%c\n", madt->base.signature[0], madt->base.signature[1], madt->base.signature[2], madt->base.signature[3]);


}

void smp_driver_init(struct smp_driver *driver)
{

    modules_driver_init(&driver->base, SMP_DRIVER_TYPE);

    driver->base.start = start;

}

