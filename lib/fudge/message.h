#define MESSAGE_SIZE                    1536

struct message_header
{

    unsigned int event;
    unsigned int source;
    unsigned int length;

};

unsigned int message_datasize(struct message_header *header);
void message_initheader(struct message_header *header, unsigned int event, unsigned int length);
