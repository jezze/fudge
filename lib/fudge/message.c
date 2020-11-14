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

unsigned int message_append(struct message_data *data, unsigned int offset, unsigned int count, void *buffer)
{

    return offset + memory_write(data->buffer, FUDGE_MSIZE, buffer, count, offset);

}

unsigned int message_appendstring(struct message_data *data, char *string, unsigned int offset)
{

    return message_append(data, offset, ascii_length(string), string);

}

unsigned int message_appendstring2(struct message_data *data, char *string, unsigned int offset)
{

    return message_append(data, offset, ascii_length(string) + 1, string);

}

unsigned int message_appendvalue(struct message_data *data, int value, unsigned int base, unsigned int padding, unsigned int offset)
{

    char num[FUDGE_NSIZE];

    return message_append(data, offset, ascii_wvalue(num, FUDGE_NSIZE, value, base, padding), num);

}

void message_initheader(struct message_header *header, unsigned int type, unsigned int length)
{

    header->type = type;
    header->source = 0;
    header->length = message_headersize(header) + length;

}

