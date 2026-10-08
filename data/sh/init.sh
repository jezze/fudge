mq -query .modules.path data/config/modules.mq | elfload
mq -query .modules.path data/config/modules-x86.mq | elfload
shell -keyboard-service keyboard:1 &
mq -query .mounts -format "automount -service {service} -index {index} -partition {partition} -name {name}" data/config/mount.mq | sh &
wm &
