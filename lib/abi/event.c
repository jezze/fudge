#include <fudge.h>
#include "file.h"
#include "event.h"

void event_open(void)
{

    file_open(FILE_PM);

}

void event_close(void)
{

    file_close(FILE_PM);

}

void event_read(union event_message *message)
{

    file_readall(FILE_PM, &message->header, sizeof (struct event_header));
    file_readall(FILE_PM, &message->header + 1, message->header.length - sizeof (struct event_header));

}

unsigned int event_send(struct event_header *header)
{

    return file_writeall(FILE_PM, header, header->length);

}

unsigned int event_send2(union event_message *message)
{

    return file_writeall(FILE_PM, message, message->header.length);

}

