#define RECORD_NAMESIZE                 64
#define RECORD_TYPE_NORMAL              0
#define RECORD_TYPE_DIRECTORY           1

struct record
{

    unsigned int id;
    unsigned int length;
    unsigned int size;
    unsigned int type;
    char name[RECORD_NAMESIZE];

};
