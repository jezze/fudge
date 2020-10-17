#include "define.h"
#include "memory.h"
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

void message_append(union message *message, unsigned int count, void *buffer)
{

    message->header.length += memory_write(message->data, FUDGE_BSIZE, buffer, count, message->header.length);

}

void message_appendstring(union message *message, char *string)
{

    message_append(message, ascii_length(string), string);

}

void message_appendstring2(union message *message, char *string)
{

    message_append(message, ascii_length(string) + 1, string);

}

void message_appendvalue(union message *message, int value, unsigned int base, unsigned int padding)
{

    char num[FUDGE_NSIZE];

    message_append(message, ascii_wvalue(num, FUDGE_NSIZE, value, base, padding), num);

}

void message_initheader(struct message_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->length = message_headersize(header) + length;

}

void message_init(union message *message, unsigned int type)
{

    message_initheader(&message->header, type, 0);

}

