#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

    union Nsipc *packet = (union Nsipc*)REQVA;

    while (1) {
        if (NSREQ_OUTPUT == ipc_recv(NULL, packet, NULL)) {
            while (0 != sys_net_send(packet->pkt.jp_data, packet->pkt.jp_len))
                sys_yield();
        }
    }
}
