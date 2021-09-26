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

struct message
{

    struct message_header header;
    struct message_data data;

};

unsigned int message_headersize(struct message_header *header);
unsigned int message_datasize(struct message_header *header);
unsigned int message_putbuffer2(struct message *message, unsigned int count, void *buffer);
unsigned int message_putstring2(struct message *message, char *string);
unsigned int message_putstringz2(struct message *message, char *string);
unsigned int message_putvalue2(struct message *message, int value, unsigned int base, unsigned int padding);
void message_initheader(struct message_header *header, unsigned int event, unsigned int length);
void message_init(struct message *message, unsigned int event);
