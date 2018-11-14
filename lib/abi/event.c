#include <fudge.h>
#include "call.h"
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

    while (!call_pick(&message->header, sizeof (struct event_header)));

    if (message->header.length > sizeof (struct event_header))
        while (!call_pick(&message->header + 1, message->header.length - sizeof (struct event_header)));

    return message->header.type;

}

unsigned int event_send(union event_message *message)
{

    return file_writeall(FILE_PM, message, message->header.length);

}

