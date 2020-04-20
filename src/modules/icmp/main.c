#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>
#include "icmp.h"

static struct ipv4_hook ipv4hook;
static struct ipv6_hook ipv6hook;

static void *icmp_writehead(void *buffer, unsigned char type, unsigned char code, unsigned char *sip, unsigned char *tip, unsigned int count, void *payload)
{

    struct icmp_header *header = ipv4_writehead(buffer, sip, tip, ipv4hook.id, count);
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

static void ipv4hook_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

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

    kernel_notify(&ipv4hook.data.states, EVENT_DATA, buffer, count);

}

static void ipv6hook_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    kernel_notify(&ipv6hook.data.states, EVENT_DATA, buffer, count);

}

void module_init(void)
{

    ipv4_inithook(&ipv4hook, "icmp", 0x01, ipv4hook_notify);
    ipv6_inithook(&ipv6hook, "icmp", 0x01, ipv6hook_notify);

}

void module_register(void)
{

    ipv4_registerhook(&ipv4hook);
    ipv6_registerhook(&ipv6hook);

}

void module_unregister(void)
{

    ipv4_unregisterhook(&ipv4hook);
    ipv6_unregisterhook(&ipv6hook);

}

