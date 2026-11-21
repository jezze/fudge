#define TLS_SEEDSIZE                    32

struct tls_connection
{

    unsigned int ethernet;
    struct socket *local;
    struct socket *remote;
    struct socket *router;
    char *servername;
    unsigned int unixtime;
    unsigned char seed[TLS_SEEDSIZE];
    void (*ondata)(unsigned int count, void *data);

};

unsigned int tls_loadanchors(unsigned int source, char *path);
unsigned int tls_request(struct tls_connection *connection, unsigned int count, void *data);
