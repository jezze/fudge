#define VIRTIO_VENDOR                   0x1AF4
#define VIRTIO_REG_HOSTFEATURES         0x00
#define VIRTIO_REG_GUESTFEATURES        0x04
#define VIRTIO_REG_QUEUEADDRESS         0x08
#define VIRTIO_REG_QUEUESIZE            0x0C
#define VIRTIO_REG_QUEUESELECT          0x0E
#define VIRTIO_REG_QUEUENOTIFY          0x10
#define VIRTIO_REG_STATUS               0x12
#define VIRTIO_REG_ISR                  0x13
#define VIRTIO_REG_CONFIGVECTOR         0x14
#define VIRTIO_REG_QUEUEVECTOR          0x16
#define VIRTIO_REG_CONFIG               0x14
#define VIRTIO_STATUS_ACKNOWLEDGE       0x01
#define VIRTIO_STATUS_DRIVER            0x02
#define VIRTIO_STATUS_DRIVEROK          0x04
#define VIRTIO_STATUS_FAILED            0x80
#define VIRTIO_DESC_NEXT                0x01
#define VIRTIO_DESC_WRITE               0x02
#define VIRTIO_NOVECTOR                 0xFFFF
#define VIRTIO_QUEUESIZE                256
#define VIRTIO_QUEUEMEMORY              0x4000

struct virtio_desc
{

    unsigned int address;
    unsigned int addresshigh;
    unsigned int length;
    unsigned short flags;
    unsigned short next;

};

struct virtio_ringhead
{

    unsigned short flags;
    unsigned short index;

};

struct virtio_usedelement
{

    unsigned int id;
    unsigned int length;

};

struct virtio_queue
{

    unsigned short io;
    unsigned short index;
    unsigned short size;
    unsigned short lastused;
    volatile struct virtio_desc *descs;
    volatile struct virtio_ringhead *avail;
    volatile unsigned short *availring;
    volatile struct virtio_ringhead *used;
    volatile struct virtio_usedelement *usedring;

};

void virtio_reset(unsigned short io);
unsigned int virtio_negotiate(unsigned short io, unsigned int features);
void virtio_setstatus(unsigned short io, unsigned char status);
unsigned int virtio_setqueue(struct virtio_queue *queue, unsigned short io, unsigned short index, void *memory);
void virtio_setdesc(struct virtio_queue *queue, unsigned int index, void *buffer, unsigned int length, unsigned short flags, unsigned short next);
void virtio_push(struct virtio_queue *queue, unsigned short head);
void virtio_notify(struct virtio_queue *queue);
unsigned int virtio_pending(struct virtio_queue *queue);
unsigned int virtio_pop(struct virtio_queue *queue, unsigned int *length);
unsigned int virtio_setinterrupt(unsigned int id, unsigned short io, unsigned int queues, void (*routine)(unsigned int irq));
void virtio_unsetinterrupt(unsigned int id, unsigned int msi);
