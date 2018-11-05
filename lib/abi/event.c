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

struct event_header *event_read(void *message)
{

    struct event_header *header = message;

    file_readall(FILE_PM, header, sizeof (struct event_header));
    file_readall(FILE_PM, header + 1, header->length - sizeof (struct event_header));

    return header;

}

unsigned int event_send(struct event_header *header)
{

    return file_writeall(FILE_PM, header, header->length);

}

