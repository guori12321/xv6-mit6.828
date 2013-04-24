#ifndef JOS_KERN_E100_H
#define JOS_KERN_E100_H

#include	<inc/types.h>
#include	<kern/pci.h>

#define PACKET_MAX_LEN 1524
#define TCB_MAX_NUM 24 


struct cb {
    volatile uint16_t status;
    uint16_t cmd;
    uint32_t link;
};

union Tcb {
    struct tcb_transmit {
        struct cb cb;
        uint32_t tbdarr;
        uint16_t tcbbc;
        uint8_t thrs;
        uint8_t tbdcount;
        char data[PACKET_MAX_LEN];
    } tcb_transmit;

    struct tcb_recieve {
        struct cb cb;
        uint32_t reserve;
        uint16_t actualcount;
        uint16_t size;
        char data[PACKET_MAX_LEN];
    } tcb_recieve;
};

extern union Tcb *cbl_ka;
extern union Tcb *rfa_ka;
extern union Tcb *noptcb_cu;
extern union Tcb *endtcb_cu;
extern union Tcb *noptcb_ru;
extern union Tcb *tcb_ru;
extern union Tcb *tcb_cu;
extern uint32_t baseio;

int e100_attach(struct pci_func *);
union Tcb * e100_alloc_tcb(size_t n, void (*)(union Tcb *));
union Tcb * e100_next_tcb(union Tcb *tcb);
union Tcb * e100_prev_tcb(union Tcb *tcb);
bool e100_tcb_complete(uint16_t status);

uint16_t e100_read_status();
void e100_wait_cmd();
uint16_t e100_read_cmd();
void e100_write_scbgp(uint32_t gp);
uint32_t e100_read_port();
void e100_write_port(uint32_t portcmd);

void e100_write_cmd_cu(uint16_t cmd);
void e100_write_cmd_ru(uint16_t cmd);
void e100_init_tcb_cu(union Tcb *tcb);
void e100_init_tcb_ru(union Tcb *tcb);
int  e100_ru_resault(uint16_t count);
int  e100_ru_count(uint16_t count);
bool e100_ru_sus(uint16_t status);
void e100_ru_res(union Tcb *tcb);
void e100_cu_res(union Tcb *tcb);
void e100_cu_int(union Tcb *tcb);
uint16_t e100_cu_status();
uint16_t e100_ru_status();

#define TCB_CMD_CS (0x01<<4)
#define TCB_CMD_CC (0x01<<5)
#define TCB_CMD_TRANS 0x0F04
#define TCB_CMD_NOP  0x4000
#define TCB_CMD_RI  0x4000
#define TCB_CMD_CI  0x4000
#define TCB_CMD_RS 0x01
#define TCB_CMD_RC 0x02

#define TCB_MASK_S (0x01<<14)

#define TCB_STATUS_IDLE 0
#define TCB_STATUS_SUSPENDED 2

#endif	// JOS_KERN_E100_H
