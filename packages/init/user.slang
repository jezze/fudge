/bin/modload < /boot/mod/base.ko
/bin/modload < /boot/mod/platform.ko
/bin/modload < /boot/mod/io.ko
/bin/modload < /boot/mod/cpuid.ko
/bin/modload < /boot/mod/msr.ko
/bin/modload < /boot/mod/pic.ko
/bin/modload < /boot/mod/pci.ko
/bin/modload < /boot/mod/pit.ko
/bin/modload < /boot/mod/rtc.ko
/bin/modload < /boot/mod/ps2.ko
/bin/modload < /boot/mod/vga.ko
/bin/modload < /boot/mod/uart.ko
/bin/modload < /boot/mod/rtl8139.ko
/bin/keymapus > /system/keyboard/dev/ps2/keymap
/bin/motd > /system/terminal/dev/pci/data
/bin/shell < /system/keyboard/dev/ps2/data > /system/terminal/dev/pci/data
/bin/motd > /system/terminal/dev/platform/data
/bin/shell < /system/terminal/dev/platform/data > /system/terminal/dev/platform/data
