#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
    binaryname = "ns_input";

    // LAB 6: Your code here:
    // 	- read a packet from the device driver
    //	- send it to the network server
    // Hint: When you IPC a page to the network server, it will be
    // reading from it for a while, so don't immediately receive
    // another packet in to the same physical page.

    union Nsipc *packet;
    int r;
    int i;
    for (i = 0; i < QUEUE_SIZE; ++i) {
        packet = (union Nsipc *)(i * PGSIZE + REQVA);
        if (0 != (r = sys_page_alloc(env->env_id, packet, PTE_U | PTE_W | PTE_P)))
            panic("ns_input: %e!\n", r);
    }

    packet = (union Nsipc *)REQVA;

    while (1) {
        memset(packet, 0, PGSIZE);
        packet->pkt.jp_len = PGSIZE - sizeof(int);
        while(-E_RETRY == (r = sys_net_recv(packet->pkt.jp_data, packet->pkt.jp_len)))
            sys_yield();

        if (0 == r)
            continue;

        packet->pkt.jp_len = r;
        ipc_send(ns_envid, NSREQ_INPUT, packet, PTE_W | PTE_U | PTE_P);
        packet = (union Nsipc *)((uint32_t)packet + PGSIZE);
        if ((union Nsipc *)(REQVA + QUEUE_SIZE * PGSIZE) == packet)
            packet = (union Nsipc *)REQVA;
    }
}
