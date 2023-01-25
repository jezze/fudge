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

unsigned int message_datasize(struct message_header *header);
unsigned int message_putbuffer(struct message *message, unsigned int count, void *buffer);
unsigned int message_putstring(struct message *message, char *string);
unsigned int message_putzero(struct message *message);
unsigned int message_putvalue(struct message *message, int value, unsigned int base, unsigned int padding);
unsigned int message_putfmt0(struct message *message, char *fmt);
unsigned int message_putfmt1(struct message *message, char *fmt, void *value1);
unsigned int message_putfmt2(struct message *message, char *fmt, void *value1, void *value2);
unsigned int message_putfmt3(struct message *message, char *fmt, void *value1, void *value2, void *value3);
unsigned int message_putfmt4(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4);
unsigned int message_putfmt6(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4, void *value5, void *value6);
unsigned int message_putfmt8(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4, void *value5, void *value6, void *value7, void *value8);
void message_initheader(struct message_header *header, unsigned int event, unsigned int length);
void message_init(struct message *message, unsigned int event);
