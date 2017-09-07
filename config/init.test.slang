initfs
ipsetup
echo < /system/log/info > /system/console/if:0/odata
elfload < /mod/acpi.ko
elfload < /mod/apic.ko
elfload < /mod/ioapic.ko
elfload < /mod/smp.ko
