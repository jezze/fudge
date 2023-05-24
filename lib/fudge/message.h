#define MESSAGE_SIZE                    1536

struct message
{

    unsigned int event;
    unsigned int source;
    unsigned int length;

};

unsigned int message_datasize(struct message *message);
void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length);
