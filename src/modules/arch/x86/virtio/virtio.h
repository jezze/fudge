#define VIRTIO_PCIVENDOR                0x1AF4
#define VIRTIO_TYPENETWORK              1
#define VIRTIO_TYPEBLOCK                2
#define VIRTIO_TYPECONSOLE              3
#define VIRTIO_TYPESCSI                 8
#define VIRTIO_TYPE9P                   9
#define VIRTIO_TYPEWLAN                 10
#define VIRTIO_TYPEGPU                  16
#define VIRTIO_TYPECLOCK                17
#define VIRTIO_TYPEINPUT                18
#define VIRTIO_TYPESOCKET               19
#define VIRTIO_TYPECRYPTO               20

#define VIRTIO_REGISTERDEVFEATURES      0x00
#define VIRTIO_REGISTERGUESTFEATURES    0x04
#define VIRTIO_REGISTERQADDRESS         0x08
#define VIRTIO_REGISTERQSIZE            0x0C
#define VIRTIO_REGISTERQSELECT          0x0E
#define VIRTIO_REGISTERQNOTIFY          0x10
#define VIRTIO_REGISTERSTATUS           0x12
#define VIRTIO_REGISTERISR              0x13
#define VIRTIO_STATUSACKNOWLEDGE        0x01
#define VIRTIO_STATUSDRIVER             0x02
#define VIRTIO_STATUSREADY              0x04
#define VIRTIO_STATUSFEATURES           0x08

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
