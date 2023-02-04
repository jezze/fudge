#include "message.h"

unsigned int message_datasize(struct message *message)
{

    return message->length - sizeof (struct message);

}

void message_dropdata(struct message *message)
{

    message->status = MESSAGE_STATUS_LOSTDATA;
    message->length = sizeof (struct message);

}

void message_init(struct message *message, unsigned int event, unsigned int length)
{

    message->event = event;
    message->source = 0;
    message->status = MESSAGE_STATUS_NORMAL;
    message->length = sizeof (struct message) + length;

}

