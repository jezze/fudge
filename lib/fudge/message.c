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

unsigned int message_putfmt0(struct message *message, char *fmt)
{

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 0, 0, message_datasize(&message->header));

}

unsigned int message_putfmt1(struct message *message, char *fmt, void *value1)
{

    void *args[1];

    args[0] = value1;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 1, args, message_datasize(&message->header));

}

unsigned int message_putfmt2(struct message *message, char *fmt, void *value1, void *value2)
{

    void *args[2];

    args[0] = value1;
    args[1] = value2;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 2, args, message_datasize(&message->header));

}

unsigned int message_putfmt3(struct message *message, char *fmt, void *value1, void *value2, void *value3)
{

    void *args[3];

    args[0] = value1;
    args[1] = value2;
    args[2] = value3;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 3, args, message_datasize(&message->header));

}

unsigned int message_putfmt4(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4)
{

    void *args[4];

    args[0] = value1;
    args[1] = value2;
    args[2] = value3;
    args[3] = value4;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 4, args, message_datasize(&message->header));

}

unsigned int message_putfmt6(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4, void *value5, void *value6)
{

    void *args[6];

    args[0] = value1;
    args[1] = value2;
    args[2] = value3;
    args[3] = value4;
    args[4] = value5;
    args[5] = value6;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 6, args, message_datasize(&message->header));

}

unsigned int message_putfmt8(struct message *message, char *fmt, void *value1, void *value2, void *value3, void *value4, void *value5, void *value6, void *value7, void *value8)
{

    void *args[8];

    args[0] = value1;
    args[1] = value2;
    args[2] = value3;
    args[3] = value4;
    args[4] = value5;
    args[5] = value6;
    args[6] = value7;
    args[7] = value8;

    return message->header.length += cstring_writefmt(message->data.buffer, MESSAGE_SIZE, fmt, 8, args, message_datasize(&message->header));

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

