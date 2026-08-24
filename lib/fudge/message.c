#include "message.h"

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length, unsigned int offset)
{

    message->event = event;
    message->source = source;
    message->length = length;
    message->offset = offset;

}

