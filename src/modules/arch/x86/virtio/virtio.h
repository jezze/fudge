#define VIRTIO_PCIVENDOR                0x1AF4
#define VIRTIO_TYPE_NETWORK             1
#define VIRTIO_TYPE_BLOCK               2
#define VIRTIO_TYPE_CONSOLE             3
#define VIRTIO_TYPE_SCSI                8
#define VIRTIO_TYPE_9P                  9
#define VIRTIO_TYPE_WLAN                10
#define VIRTIO_TYPE_GPU                 16
#define VIRTIO_TYPE_CLOCK               17
#define VIRTIO_TYPE_INPUT               18
#define VIRTIO_TYPE_SOCKET              19
#define VIRTIO_TYPE_CRYPTO              20
#define VIRTIO_REG_DEVFEATURES          0x00
#define VIRTIO_REG_GUESTFEATURES        0x04
#define VIRTIO_REG_QADDRESS             0x08
#define VIRTIO_REG_QSIZE                0x0C
#define VIRTIO_REG_QSELECT              0x0E
#define VIRTIO_REG_QNOTIFY              0x10
#define VIRTIO_REG_STATUS               0x12
#define VIRTIO_REG_STATUS_ACKNOWLEDGE   0x01
#define VIRTIO_REG_STATUS_DRIVER        0x02
#define VIRTIO_REG_STATUS_READY         0x04
#define VIRTIO_REG_STATUS_FEATURES      0x08
#define VIRTIO_REG_ISR                  0x13

struct virtio_buffer
{

    unsigned int address;
    unsigned int haddress;
    unsigned int length;
    unsigned short flags;
    unsigned short next;

};

struct virtio_availablehead
{

    unsigned short flags;
    unsigned short index;

};

struct virtio_availablering
{

    unsigned short index;

};

struct virtio_usedhead
{

    unsigned short flags;
    unsigned short index;

};

struct virtio_usedring
{

    unsigned int index;
    unsigned int length;

};

struct virtio_queue
{

    unsigned short size;
    unsigned int address;
    struct virtio_buffer *buffers;
    unsigned int bufferssize;
    struct virtio_availablehead *availablehead;
    struct virtio_availablering *availablering;
    unsigned int availablesize;
    struct virtio_usedhead *usedhead;
    struct virtio_usedring *usedring;
    unsigned int usedsize;
    unsigned int lastindex;
    unsigned int numbuffers;

};

void virtio_reset(unsigned short io);
void virtio_setqueue(unsigned short io, unsigned short index, struct virtio_queue *vq, unsigned int address);
void virtio_setrx(unsigned short io, struct virtio_queue *vq, void *buffer);
