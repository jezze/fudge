#define MESSAGE_SIZE                    1536

struct message_header
{

    unsigned int event;
    unsigned int source;
    unsigned int length;

};

struct message_data
{

    char buffer[MESSAGE_SIZE];

};

unsigned int message_headersize(struct message_header *header);
unsigned int message_datasize(struct message_header *header);
unsigned int message_putbuffer(struct message_data *data, unsigned int count, void *buffer, unsigned int offset);
unsigned int message_putstring(struct message_data *data, char *string, unsigned int offset);
unsigned int message_putstringz(struct message_data *data, char *string, unsigned int offset);
unsigned int message_putvalue(struct message_data *data, int value, unsigned int base, unsigned int padding, unsigned int offset);
void message_initheader(struct message_header *header, unsigned int event, unsigned int source, unsigned int length);
