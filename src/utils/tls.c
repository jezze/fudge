#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>
#include <bearssl.h>
#include "tls.h"

#define ANCHORS                         160
#define POOLSIZE                        0x10000
#define DNSIZE                          512
#define UNIXDAYS                        719528
#define DAYSECONDS                      86400

static br_ssl_client_context client;
static br_x509_minimal_context x509;
static unsigned char iobuffer[BR_SSL_BUFSIZE_BIDI];
static unsigned char received[MESSAGE_SIZE];
static br_x509_trust_anchor anchors[ANCHORS];
static unsigned int nanchors;
static unsigned char pool[POOLSIZE];
static unsigned int poolcount;
static br_pem_decoder_context pem;
static br_x509_decoder_context decoder;
static unsigned char dn[DNSIZE];
static unsigned int dncount;
static unsigned int dnfull;
static unsigned int incert;

static unsigned char *store(const unsigned char *data, unsigned int count)
{

    unsigned char *out = pool + poolcount;

    if (count > POOLSIZE - poolcount)
        return 0;

    buffer_copy(out, (void *)data, count);

    poolcount += count;

    return out;

}

static void appenddn(void *context, const void *data, size_t count)
{

    if (count > DNSIZE - dncount)
        dnfull = 1;
    else
        dncount += buffer_write(dn, DNSIZE, (void *)data, count, dncount);

}

static void pushcert(void *context, const void *data, size_t count)
{

    br_x509_decoder_push(&decoder, data, count);

}

static void addanchor(void)
{

    br_x509_pkey *pkey = br_x509_decoder_get_pkey(&decoder);
    br_x509_trust_anchor *anchor;

    if (nanchors >= ANCHORS || dnfull || !pkey || br_x509_decoder_last_error(&decoder))
        return;

    anchor = &anchors[nanchors];

    anchor->dn.data = store(dn, dncount);
    anchor->dn.len = dncount;
    anchor->flags = BR_X509_TA_CA;
    anchor->pkey.key_type = pkey->key_type;

    if (!anchor->dn.data)
        return;

    if (pkey->key_type == BR_KEYTYPE_RSA)
    {

        anchor->pkey.key.rsa.n = store(pkey->key.rsa.n, pkey->key.rsa.nlen);
        anchor->pkey.key.rsa.nlen = pkey->key.rsa.nlen;
        anchor->pkey.key.rsa.e = store(pkey->key.rsa.e, pkey->key.rsa.elen);
        anchor->pkey.key.rsa.elen = pkey->key.rsa.elen;

        if (anchor->pkey.key.rsa.n && anchor->pkey.key.rsa.e)
            nanchors++;

    }

    if (pkey->key_type == BR_KEYTYPE_EC)
    {

        anchor->pkey.key.ec.curve = pkey->key.ec.curve;
        anchor->pkey.key.ec.q = store(pkey->key.ec.q, pkey->key.ec.qlen);
        anchor->pkey.key.ec.qlen = pkey->key.ec.qlen;

        if (anchor->pkey.key.ec.q)
            nanchors++;

    }

}

static void onpemdata(unsigned int source, void *buffer, unsigned int count)
{

    unsigned char *data = buffer;

    while (count)
    {

        unsigned int pushed = br_pem_decoder_push(&pem, data, count);

        data += pushed;
        count -= pushed;

        switch (br_pem_decoder_event(&pem))
        {

        case BR_PEM_BEGIN_OBJ:
            incert = cstring_match((char *)br_pem_decoder_name(&pem), "CERTIFICATE");
            dncount = 0;
            dnfull = 0;

            br_x509_decoder_init(&decoder, appenddn, 0);
            br_pem_decoder_setdest(&pem, pushcert, 0);

            break;

        case BR_PEM_END_OBJ:
            if (incert)
                addanchor();

            incert = 0;

            break;

        case BR_PEM_ERROR:
            incert = 0;

            br_pem_decoder_init(&pem);

            break;

        }

    }

}

unsigned int tls_loadanchors(unsigned int source, char *path)
{

    nanchors = 0;
    poolcount = 0;
    incert = 0;

    br_pem_decoder_init(&pem);
    fs_read_each(1, source, path, onpemdata);

    return nanchors;

}

unsigned int tls_request(struct tls_connection *connection, unsigned int count, void *data)
{

    unsigned char *request = data;
    br_ssl_engine_context *engine = &client.eng;
    unsigned int sent = 0;
    unsigned int pendingoffset = 0;
    unsigned int pendingcount = 0;

    br_ssl_client_init_full(&client, &x509, anchors, nanchors);
    br_x509_minimal_set_time(&x509, connection->unixtime / DAYSECONDS + UNIXDAYS, connection->unixtime % DAYSECONDS);
    br_ssl_engine_set_buffer(engine, iobuffer, BR_SSL_BUFSIZE_BIDI, 1);
    br_ssl_engine_inject_entropy(engine, connection->seed, TLS_SEEDSIZE);

    if (!br_ssl_client_reset(&client, connection->servername, 0))
        return br_ssl_engine_last_error(engine);

    for (;;)
    {

        unsigned int state = br_ssl_engine_current_state(engine);
        unsigned char *buffer;
        size_t length;

        if (state & BR_SSL_CLOSED)
            break;

        if (state & BR_SSL_SENDREC)
        {

            buffer = br_ssl_engine_sendrec_buf(engine, &length);

            socket_send_tcp(0, connection->ethernet, connection->local, connection->remote, connection->router, length, buffer);
            br_ssl_engine_sendrec_ack(engine, length);

            continue;

        }

        if (state & BR_SSL_RECVAPP)
        {

            buffer = br_ssl_engine_recvapp_buf(engine, &length);

            connection->ondata(length, buffer);
            br_ssl_engine_recvapp_ack(engine, length);

            continue;

        }

        if (sent < count && (state & BR_SSL_SENDAPP))
        {

            buffer = br_ssl_engine_sendapp_buf(engine, &length);

            if (length > count - sent)
                length = count - sent;

            buffer_copy(buffer, request + sent, length);
            br_ssl_engine_sendapp_ack(engine, length);

            sent += length;

            if (sent == count)
                br_ssl_engine_flush(engine, 0);

            continue;

        }

        if (state & BR_SSL_RECVREC)
        {

            if (!pendingcount)
            {

                if (connection->remote->info.tcp.state != TCP_STATE_ESTABLISHED)
                    break;

                pendingoffset = 0;
                pendingcount = socket_receive(0, connection->ethernet, connection->local, connection->remote, 1, connection->router, received, MESSAGE_SIZE);

                if (!pendingcount)
                    break;

            }

            buffer = br_ssl_engine_recvrec_buf(engine, &length);

            if (length > pendingcount)
                length = pendingcount;

            buffer_copy(buffer, received + pendingoffset, length);
            br_ssl_engine_recvrec_ack(engine, length);

            pendingoffset += length;
            pendingcount -= length;

            continue;

        }

        break;

    }

    return br_ssl_engine_last_error(engine);

}

