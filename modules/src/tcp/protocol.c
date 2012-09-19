#include <memory.h>
#include <vfs.h>
#include <net/net.h>
#include <tcp/tcp.h>

void tcp_protocol_init(struct tcp_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct tcp_protocol));

    protocol->base.name = "tcp";
/*    protocol->read_data = read_data; */
/*    protocol->write_data = write_data; */

}

