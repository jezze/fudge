#include "message.h"

void *message_data(struct message *message, unsigned int ichannel)
{

    return (void *)(message->data + MESSAGE_SIZE * MESSAGE_SLOTS * ichannel);

}

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, unsigned long data)
{

    message->event = event;
    message->source = source;
    message->length = length;
    message->data = data;

}

