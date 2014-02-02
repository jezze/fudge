/bin/dep < /config/initmod.conf | /bin/initmod
/bin/echo < /system/keyboard/clone
/bin/echo < /system/terminal/clone
/bin/keymapus > /system/keyboard/1/device/keymap
/bin/motd > /system/terminal/1/device/data
/bin/shell < /system/keyboard/1/device/data << /system/null > /system/terminal/1/device/data >> /system/null
