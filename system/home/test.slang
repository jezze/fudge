echo > /system/video/vga/resolution "320x200x8"
pcxcmap < /data/mi.pcx | cconv8to6 > /system/video/vga/colormap
pcxdata < /data/mi.pcx > /system/video/vga/data
