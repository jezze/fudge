initfs
ipsetup
echo < /system/log/info > /system/console/if:1/data
elfload < /mod/virtio-network.ko.0
elfload < /mod/acpi.ko
elfload < /mod/apic.ko
elfload < /mod/smp.ko
slang < /config/wm.slang
