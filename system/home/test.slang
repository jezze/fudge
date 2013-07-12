echo > /system/video/vga/mode "320x200x8"
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/vga/colormap
pcxdata < /share/icons.pcx > /system/video/vga/data
