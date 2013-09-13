echo < "320x200x8" > /system/video/0000/video/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/0000/video/colormap
pcxdata < /share/icons.pcx > /system/video/0000/video/data
