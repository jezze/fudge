/bin/modload < /boot/mod/base.ko.0
/bin/modload < /boot/mod/platform.ko.0
/bin/modload < /boot/mod/io.ko.0
/bin/modload < /boot/mod/cpuid.ko.0
/bin/modload < /boot/mod/msr.ko.0
/bin/modload < /boot/mod/pic.ko.0
/bin/modload < /boot/mod/pci.ko.0
/bin/modload < /boot/mod/pit.ko.0
/bin/modload < /boot/mod/rtc.ko.0
/bin/modload < /boot/mod/ps2.ko.0
/bin/modload < /boot/mod/vga.ko.0
/bin/modload < /boot/mod/uart.ko.0
/bin/modload < /boot/mod/rtl8139.ko.0
/bin/keymapus > /system/keyboard/ps2/keymap
/bin/motd > /system/terminal/pci/data
/bin/shell < /system/keyboard/ps2/data > /system/terminal/pci/data
/bin/motd > /system/terminal/platform/data
/bin/shell < /system/terminal/platform/data > /system/terminal/platform/data
