#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/mmu.h>
#include <kernel/x86/arch.h>
#include <modules/system/system.h>
#include <modules/arch/x86/acpi/acpi.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/ioapic/ioapic.h>
#include <modules/arch/x86/pit/pit.h>
#include "smp.h"

void module_init(void)
{

    apic_sendint(0, APIC_ICR_TYPE_NORMAL | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0xFE);
    apic_sendint(1, APIC_ICR_TYPE_NORMAL | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0xFE);
    apic_sendint(2, APIC_ICR_TYPE_NORMAL | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0xFE);
    apic_sendint(3, APIC_ICR_TYPE_NORMAL | APIC_ICR_MODE_PHYSICAL | APIC_ICR_LEVEL_ASSERT | APIC_ICR_TRIGGER_EDGE | APIC_ICR_TARGET_NORMAL | 0xFE);

}

