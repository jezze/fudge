echo > /system/video/vga/mode "320x200x8"
pcxcmap < /data/icons.pcx | cconv8to6 > /system/video/vga/colormap
pcxdata < /data/icons.pcx > /system/video/vga/data
