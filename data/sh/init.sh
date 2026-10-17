mq -query .modules.path data/config/modules.mq | elfload
mq -query .modules.path data/config/modules-arch.mq | elfload
shell -keyboard-service keyboard:1 &
mq -query .mounts -format "automount -service {service} -index {index} -partition {partition} -name {name}" data/config/mount.mq | sh &
wm &
