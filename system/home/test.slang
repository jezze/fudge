echo < "320x200x8" > /system/video/vga/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/vga/colormap
pcxdata < /share/icons.pcx > /system/video/vga/data
