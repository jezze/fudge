#include "buffer.h"
#include "ascii.h"
#include "message.h"

unsigned int message_headersize(struct message_header *header)
{

    return sizeof (struct message_header);

}

unsigned int message_datasize(struct message_header *header)
{

    return header->length - message_headersize(header);

}

unsigned int message_putbuffer(struct message_data *data, unsigned int count, void *buffer, unsigned int offset)
{

    return offset + buffer_write(data->buffer, MESSAGE_SIZE, buffer, count, offset);

}

unsigned int message_putstring(struct message_data *data, char *string, unsigned int offset)
{

    return message_putbuffer(data, ascii_length(string), string, offset);

}

unsigned int message_putstringz(struct message_data *data, char *string, unsigned int offset)
{

    return message_putbuffer(data, ascii_lengthz(string), string, offset);

}

unsigned int message_putvalue(struct message_data *data, int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[ASCII_NUMSIZE];

    return message_putbuffer(data, ascii_wvalue(num, ASCII_NUMSIZE, value, base, padding), num, offset);

}

void message_initheader(struct message_header *header, unsigned int event, unsigned int length)
{

    header->event = event;
    header->source = 0;
    header->length = message_headersize(header) + length;

}

