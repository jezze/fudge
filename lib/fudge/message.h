struct message_header
{

    unsigned int type;
    unsigned int source;
    unsigned int length;

};

struct message_data
{

    char data[FUDGE_MSIZE];

};

unsigned int message_headersize(struct message_header *header);
unsigned int message_datasize(struct message_header *header);
unsigned int message_append(struct message_data *message, unsigned int offset, unsigned int count, void *buffer);
unsigned int message_appendstring(struct message_data *message, char *string, unsigned int offset);
unsigned int message_appendstring2(struct message_data *message, char *string, unsigned int offset);
unsigned int message_appendvalue(struct message_data *message, int value, unsigned int base, unsigned int padding, unsigned int offset);
void message_initheader(struct message_header *header, unsigned int type, unsigned int length);
