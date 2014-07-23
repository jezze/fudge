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
/bin/keymapus > /system/keyboard/ps2/keymap
/bin/motd > /system/terminal/pci/data
/bin/shell < /system/keyboard/ps2/data > /system/terminal/pci/data
/bin/motd > /system/terminal/platform/data
/bin/shell < /system/terminal/platform/data > /system/terminal/platform/data
