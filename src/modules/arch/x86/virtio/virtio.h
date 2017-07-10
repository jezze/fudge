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
