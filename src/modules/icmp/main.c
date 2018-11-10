#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "icmp.h"

static struct ipv4_protocol ipv4protocol;
static struct ipv6_protocol ipv6protocol;

static void *icmp_writehead(void *buffer, unsigned char type, unsigned char code, unsigned char *sip, unsigned char *tip, unsigned int count, void *payload)
{

    struct icmp_header *header = ipv4_writehead(buffer, sip, tip, ipv4protocol.id, count);
    unsigned int checksum;

    memory_copy(header, payload, count);

    header->type = type;
    header->code = code;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    checksum = ipv4_calculatechecksum(header, count);

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

    return header + count;

}

static void ipv4protocol_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    union event_message message;
    struct icmp_header *header = buffer;

    switch (header->type)
    {

    case ICMP_ECHOREQUEST:
        {

            unsigned char response[0x2000];
            unsigned char *current = icmp_writehead(response, ICMP_ECHOREPLY, 0, ipv4header->tip, ipv4header->sip, count, buffer);

            ipv4_send(response, current - response);

        }

    }

    event_createdata(&message.header, EVENT_BROADCAST, 0);
    event_appenddata(&message.header, count, buffer);
    kernel_multicast(&ipv4protocol.data.states, &message);

}

static void ipv6protocol_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    union event_message message;

    event_createdata(&message.header, EVENT_BROADCAST, 0);
    event_appenddata(&message.header, count, buffer);
    kernel_multicast(&ipv6protocol.data.states, &message);

}

void module_init(void)
{

    ipv4_initprotocol(&ipv4protocol, "icmp", 0x01, ipv4protocol_notify);
    ipv6_initprotocol(&ipv6protocol, "icmp", 0x01, ipv6protocol_notify);

}

void module_register(void)
{

    ipv4_registerprotocol(&ipv4protocol);
    ipv6_registerprotocol(&ipv6protocol);

}

void module_unregister(void)
{

    ipv4_unregisterprotocol(&ipv4protocol);
    ipv6_unregisterprotocol(&ipv6protocol);

}

