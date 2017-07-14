#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "icmp.h"

static struct ipv4_protocol ipv4protocol;
static struct ipv6_protocol ipv6protocol;

static void *icmp_writehead(void *buffer, unsigned char type, unsigned char code)
{

    struct icmp_header *header = buffer;

    header->type = type;
    header->code = code;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    return header + 1;

}

static void ipv4protocol_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    struct icmp_header *header = buffer;

    switch (header->type)
    {

    case ICMP_ECHOREQUEST:
        debug_write(DEBUG_INFO, "ICMP", "type", header->type);
        {

            unsigned char response[0x2000];
            unsigned char *current = response;
            struct icmp_header *header2;
            unsigned int checksum;

            header2 = ipv4_writehead(current, ipv4header->tip, ipv4header->sip, ipv4protocol.id, count);

            memory_copy(header2, buffer, count);

            current = icmp_writehead(header2, ICMP_ECHOREPLY, 0);
            current += count - sizeof (struct icmp_header);

            checksum = ipv4_calculatechecksum(header2, count);

            header2->checksum[0] = checksum;
            header2->checksum[1] = checksum >> 8;

            ipv4_send(response, current - response);

        }

    }

    kernel_multicast(&ipv4protocol.datastates, buffer, count);

}

static void ipv6protocol_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    kernel_multicast(&ipv6protocol.datastates, buffer, count);

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

