#include "message.h"

void *message_data(struct message *message, unsigned int ichannel)
{

    unsigned int address = 0x40000000 + 0x8000 * ichannel;

    return (void *)(address + message->offset);

}

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, unsigned int offset)
{

    message->event = event;
    message->source = source;
    message->length = length;
    message->offset = offset;

}

