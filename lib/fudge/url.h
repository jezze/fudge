#define URL_SCHEME                      1
#define URL_LIMBO1                      2
#define URL_LIMBO2                      3
#define URL_AUTHORITY                   4
#define URL_USERINFO                    5
#define URL_HOST                        6
#define URL_PORT                        7
#define URL_PATH                        8
#define URL_QUERY                       9
#define URL_FRAGMENT                    10

struct url
{

    unsigned int full;
    unsigned int authority;
    char *scheme;
    char *userinfo;
    char *host;
    char *port;
    char *path;
    char *query;
    char *fragment;

};

unsigned int url_parse(struct url *url, void *buffer, unsigned int buffersize, char *s, unsigned int state);
unsigned int url_tostring(struct url *url, char *buffer, unsigned int buffersize);
