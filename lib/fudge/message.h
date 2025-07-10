#define MESSAGE_SIZE                    1536
#define MESSAGE_RETRY                   0x01
#define MESSAGE_OK                      0x02
#define MESSAGE_FAILED                  0x03
#define MESSAGE_UNIMPLEMENTED           0x04

struct message
{

    unsigned int event;
    unsigned int source;
    unsigned int length;

};

unsigned int message_datasize(struct message *message);
void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length);
