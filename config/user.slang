/bin/motd > /system/console:0/out
/bin/slang < /config/wm.slang | /bin/wdraw
/bin/motd > /system/console:1/out
/bin/shell < /system/console:1/in > /system/console:1/out
