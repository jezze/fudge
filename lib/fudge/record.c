#include "buffer.h"
#include "record.h"

void record_init(struct record *record, unsigned int id, unsigned int type, unsigned int size, unsigned int offset, unsigned int length, char *name)
{

    record->id = id;
    record->type = type;
    record->size = size;
    record->offset = offset;
    record->length = length;

    buffer_copy(record->name, name, length);

}

