/bin/keymapus > /system/keyboard/ps2:0/keymap
/bin/motd > /system/terminal/pci:0/data
/bin/shell < /system/keyboard/ps2:0/data > /system/terminal/pci:0/data
/bin/motd > /system/terminal/platform:0/data
/bin/shell < /system/terminal/platform:0/data > /system/terminal/platform:0/data
