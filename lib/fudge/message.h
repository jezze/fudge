#define MESSAGE_SIZE                    1536
#define MESSAGE_STATUS_NORMAL           0
#define MESSAGE_STATUS_LOSTDATA         1

struct message
{

    unsigned int event;
    unsigned int source;
    unsigned int status;
    unsigned int length;

};

unsigned int message_datasize(struct message *message);
void message_dropdata(struct message *message);
void message_init(struct message *message, unsigned int event, unsigned int length);
