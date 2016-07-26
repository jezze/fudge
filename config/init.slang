/bin/slang < /config/base.slang
/bin/slang < /config/arch.slang
/bin/initfs
/bin/slang < /config/user.slang
/bin/slang < /config/wm.slang | /bin/wdraw > /system/video/if:0/
