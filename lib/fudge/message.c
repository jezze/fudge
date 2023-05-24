#include "message.h"

unsigned int message_datasize(struct message *message)
{

    return message->length - sizeof (struct message);

}

void message_init(struct message *message, unsigned int event, unsigned int source, unsigned int length)
{

    message->event = event;
    message->source = source;
    message->length = sizeof (struct message) + length;

}

