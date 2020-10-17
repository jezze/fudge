struct message_header
{

    unsigned int type;
    unsigned int source;
    unsigned int length;

};

union message
{

    struct message_header header;
    char data[FUDGE_BSIZE];

};

unsigned int message_datasize(struct message_header *header);
void message_append(union message *message, unsigned int count, void *buffer);
void message_appendstring(union message *message, char *string);
void message_appendstring2(union message *message, char *string);
void message_appendvalue(union message *message, int value, unsigned int base, unsigned int padding);
void message_initheader(struct message_header *header, unsigned int type, unsigned int length);
void message_init(union message *message, unsigned int type);
