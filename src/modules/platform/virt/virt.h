#define VIRT_TIMER                      0x0001
#define VIRT_RTC                        0x0002
#define VIRT_UART0                      0x0003
#define VIRT_VIRTIO                     0x0004
#define VIRT_VIRTIOS                    8
#define VIRT_FWCFG                      (VIRT_VIRTIO + VIRT_VIRTIOS)
#define VIRT_TTY                        (VIRT_FWCFG + 1)
