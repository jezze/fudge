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

unsigned int event_pick(union event_message *message)
{

    file_readall(FILE_PM, &message->header, sizeof (struct event_header));
    file_readall(FILE_PM, &message->header + 1, message->header.length - sizeof (struct event_header));

    return message->header.type;

}

unsigned int event_send(union event_message *message)
{

    return file_writeall(FILE_PM, message, message->header.length);

}

