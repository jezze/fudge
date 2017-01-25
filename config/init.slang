slang < /config/base.slang
slang < /config/arch.slang
initfs
ipsetup
motd > /system/console/if:1/data
shell < /system/console/if:0/data > /system/console/if:1/data
winit > /system/video/if:0/
slang < /config/wm.slang | wdraw > /system/video/if:0/
