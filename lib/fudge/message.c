#include "message.h"

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, void *data)
{

    message->event = event;
    message->source = source;
    message->length = length;
    message->data = data;

}

