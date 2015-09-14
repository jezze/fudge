#define RECORD_NAMESIZE                 64

struct record
{

    unsigned int id;
    unsigned int length;
    unsigned int size;
    char name[RECORD_NAMESIZE];

};
