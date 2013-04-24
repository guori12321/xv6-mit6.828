// LAB 6: Your driver code here

#include	<kern/e100.h>
#include	<inc/x86.h>
#include	<inc/mmu.h>
#include	<kern/pmap.h>
#include	<inc/string.h>

union Tcb *cbl_ka, *rfa_ka;
union Tcb *noptcb_cu = NULL, *tcb_cu = NULL;
union Tcb *noptcb_ru = NULL, *tcb_ru = NULL;

uint32_t baseio;

uint16_t
e100_read_status()
{
    return inw(baseio);
}

bool 
e100_tcb_complete(uint16_t status)
{
    return (status & (0x01<<15));
}

int e100_ru_resault(uint16_t count)
{
    return (count>>14);
}

int e100_ru_count(uint16_t count)
{
    return (count & (~(0x03<<14)));
}

bool 
e100_ru_sus(uint16_t status)
{
    return (0x01 == ((status>>2) & 0x01));
}

void 
e100_ru_res(union Tcb *tcb)
{
    tcb->tcb_recieve.cb.cmd &= (~TCB_MASK_S);
}

void 
e100_cu_res(union Tcb *tcb)
{
    tcb->tcb_transmit.cb.cmd &= (~TCB_MASK_S);
}

uint16_t 
e100_read_cmd()
{
    return inw(baseio + 0x02);
}

void 
e100_write_cmd_cu(uint16_t cmd)
{
    e100_wait_cmd();
    outw(baseio + 0x02, cmd);
}

void
e100_write_cmd_ru(uint16_t cmd)
{
    e100_wait_cmd();
    outw(baseio + 0x02, cmd);
}

void 
e100_write_scbgp(uint32_t gp)
{
    e100_wait_cmd();
    outl(baseio + 0x04, gp);
}

uint32_t 
e100_read_port()
{
    return inl(baseio + 0x08);
}

void 
e100_write_port(uint32_t port)
{
    outl(baseio + 0x08, port);
}

void
e100_wait_cmd()
{
    uint16_t cmd = e100_read_cmd();
     while (0 != cmd)
        cmd = e100_read_cmd();
}

uint16_t
e100_cu_status()
{
    return (e100_read_status() >> 5) & 0x03;
}

void
e100_cu_int(union Tcb *tcb)
{
    tcb->tcb_transmit.cb.cmd |= TCB_CMD_CI;
}

int
e100_attach(struct pci_func *pcif)
{
    uint32_t portcmd;

    pci_func_enable(pcif);

    baseio = pcif->reg_base[1];

    portcmd = e100_read_port();
    portcmd &= (~(0x0F));
    e100_write_port(portcmd);

    cbl_ka = e100_alloc_tcb(TCB_MAX_NUM, e100_init_tcb_cu);
    rfa_ka = e100_alloc_tcb(TCB_MAX_NUM, e100_init_tcb_ru);

    assert(NULL != cbl_ka && NULL != rfa_ka);

    return 0;
};

void 
e100_init_tcb_cu(union Tcb *tcb)
{
    tcb->tcb_transmit.cb.cmd = TCB_CMD_TRANS;
    tcb->tcb_transmit.cb.status = 0x01<<15;
    tcb->tcb_transmit.tbdarr = 0xffffffff;
    tcb->tcb_transmit.tbdcount = 0;
    tcb->tcb_transmit.thrs = 0xe0;
    tcb->tcb_transmit.tcbbc = 0;
}

void
e100_init_tcb_ru(union Tcb *tcb)
{
    tcb->tcb_recieve.cb.cmd = 0;
    tcb->tcb_recieve.cb.status = 0;
    tcb->tcb_recieve.reserve = 0xffffffff;
    tcb->tcb_recieve.actualcount = 0;
    tcb->tcb_recieve.size = PACKET_MAX_LEN & (~(0x03<<14));
}

union Tcb * 
e100_alloc_tcb(size_t n, void (*finit)(union Tcb *))
{
    extern struct Env *env;

    union Tcb *tcb = NULL, *ntcb = NULL, *tcblist = NULL;
    int i, r;

    for (i = 0; i < n; ++i) {
        if (NULL == tcb || ROUNDUP(tcb, PGSIZE) - tcb < sizeof(*tcb)) {
            struct Page *pag;
            if (0 != (r = page_alloc(&pag)))
                panic("e100_alloc_tcb: %e!\n");
            if (NULL == tcb) {
                tcb = page2kva(pag);
                memset(tcb, 0, PGSIZE);
                tcblist = tcb;
            }
            else {
                ntcb = page2kva(pag);
                tcb->tcb_transmit.cb.link = page2pa(pag);
                tcb = ntcb;
            }

        }
        else {
            ntcb = tcb + 1;
            tcb->tcb_transmit.cb.link = PADDR(tcb) + PGOFF(ntcb);
            tcb = ntcb;
        }

        finit(tcb);
    }

    ntcb->tcb_transmit.cb.link = PADDR(tcblist);

    return tcblist;
}

union Tcb * 
e100_next_tcb(union Tcb *tcb)
{
    return KADDR(tcb->tcb_transmit.cb.link);
}

union Tcb *
e100_prev_tcb(union Tcb *tcb)
{
    union Tcb *ptcb = tcb;
    union Tcb *ntcb;
    while (tcb != (ntcb = e100_next_tcb(ptcb)))
        ptcb = ntcb;
    return ptcb;
}
