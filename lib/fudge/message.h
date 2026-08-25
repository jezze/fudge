#define MESSAGE_SIZE                    0x1000
#define MESSAGE_SLOTS                   8
#define MESSAGE_CAPACITY                (MESSAGE_SIZE * MESSAGE_SLOTS)
#define MESSAGE_OK                      0x01
#define MESSAGE_RETRY                   0x02
#define MESSAGE_FAILED                  0x03
#define MESSAGE_UNIMPLEMENTED           0x04

struct message
{

    unsigned int event;
    unsigned int source;
    unsigned int length;
    unsigned long data;

};

void *message_data(struct message *message, unsigned int ichannel);
void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, unsigned long data);
