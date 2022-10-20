#include "buffer.h"
#include "cstring.h"
#include "message.h"

unsigned int message_datasize(struct message_header *header)
{

    return header->length - sizeof (struct message_header);

}

unsigned int message_putbuffer(struct message *message, unsigned int count, void *buffer)
{

    return message->header.length += buffer_write(message->data.buffer, MESSAGE_SIZE, buffer, count, message_datasize(&message->header));

}

unsigned int message_putstring(struct message *message, char *string)
{

    return message->header.length += cstring_write(message->data.buffer, MESSAGE_SIZE, string, message_datasize(&message->header));

}

unsigned int message_putzero(struct message *message)
{

    return message->header.length += cstring_writezero(message->data.buffer, MESSAGE_SIZE, message_datasize(&message->header));

}

unsigned int message_putvalue(struct message *message, int value, unsigned int base, unsigned int padding)
{

    return message->header.length += cstring_writevalue(message->data.buffer, MESSAGE_SIZE, value, base, padding, message_datasize(&message->header));

}

void message_initheader(struct message_header *header, unsigned int event, unsigned int length)
{

    header->event = event;
    header->source = 0;
    header->length = sizeof (struct message_header) + length;

}

void message_init(struct message *message, unsigned int event)
{

    message_initheader(&message->header, event, 0);

}

