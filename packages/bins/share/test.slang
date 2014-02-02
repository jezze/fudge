echo < /system/video/clone
echo < "320x200x8" > /system/video/1/device/mode
pcxcmap < /share/icons.pcx | cconv8to6 > /system/video/1/device/colormap
pcxdata < /share/icons.pcx > /system/video/1/device/data
