#define INPUT_EVENTS                    64
#define INPUT_EVSYN                     0x00
#define INPUT_EVKEY                     0x01
#define INPUT_EVREL                     0x02

struct input_event
{

    unsigned short type;
    unsigned short code;
    unsigned int value;

};

unsigned int input_match(unsigned int id, unsigned int type);
unsigned int input_setup(struct virtio_device *device, struct virtio_queue *queue, unsigned int id, void *memory, struct input_event *events);
void input_poll(struct virtio_device *device, struct virtio_queue *queue, unsigned int msi, struct input_event *events, void (*handle)(struct input_event *event));
