#define MESSAGE_SIZE                    0x1000
#define MESSAGE_SLOTS                   8
#define MESSAGE_CAPACITY                (MESSAGE_SIZE * MESSAGE_SLOTS)
#define MESSAGE_OK                      0x01
#define MESSAGE_RETRY                   0x02
#define MESSAGE_FAILED                  0x03

struct message
{

    unsigned int event;
    unsigned int source;
    unsigned int length;
    void *data;

};

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, void *data);
