/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* The TRAPHANDLER macro defines a globally-visible function for handling
 * a trap.  It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 */ 
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(f_divide, T_DIVIDE)
TRAPHANDLER_NOEC(f_debug, T_DEBUG)
TRAPHANDLER_NOEC(f_nmi, T_NMI)
TRAPHANDLER_NOEC(f_brkpt, T_BRKPT)
TRAPHANDLER_NOEC(f_oflow, T_OFLOW)
TRAPHANDLER_NOEC(f_bound, T_BOUND)
TRAPHANDLER_NOEC(f_illop, T_ILLOP)
TRAPHANDLER_NOEC(f_device, T_DEVICE)
TRAPHANDLER(f_dblflt, T_DBLFLT)
#
TRAPHANDLER(f_tss, T_TSS)
TRAPHANDLER(f_segnp, T_SEGNP)
TRAPHANDLER(f_stack, T_STACK)
TRAPHANDLER(f_gpflt, T_GPFLT)
TRAPHANDLER(f_pgflt, T_PGFLT)
#
TRAPHANDLER_NOEC(f_fperr, T_FPERR)
TRAPHANDLER(f_alifn, T_ALIGN)
TRAPHANDLER_NOEC(f_mchk, T_MCHK)
TRAPHANDLER_NOEC(f_simderr, T_SIMDERR)

#SYSCALL
TRAPHANDLER_NOEC(f_syscall, T_SYSCALL)

#IRQ
TRAPHANDLER_NOEC(f_timer, (IRQ_TIMER+IRQ_OFFSET))
TRAPHANDLER_NOEC(f_kbd, (IRQ_KBD+IRQ_OFFSET))
TRAPHANDLER_NOEC(f_serial, (IRQ_SERIAL+IRQ_OFFSET))
TRAPHANDLER_NOEC(f_spurious, (IRQ_SPURIOUS+IRQ_OFFSET))
TRAPHANDLER_NOEC(f_ide, (IRQ_IDE+IRQ_OFFSET))
TRAPHANDLER_NOEC(f_error, (IRQ_ERROR+IRQ_OFFSET))

.data
.globl vectors
vectors:
    .long f_divide
    .long f_debug
    .long f_nmi
    .long f_brkpt
    .long f_oflow
    .long f_bound
    .long f_illop
    .long f_device
    .long f_dblflt
    .long 0
    .long f_tss
    .long f_segnp
    .long f_stack
    .long f_gpflt
    .long f_pgflt
    .long 0
    .long f_fperr
    .long f_alifn
    .long f_mchk
    .long f_simderr
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long f_timer
    .long f_kbd
    .long 0
    .long 0
    .long f_serial
    .long 0
    .long 0
    .long f_spurious
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long 0
    .long f_ide
    .long 0
    .long f_syscall
    .long 0


/*
 * Lab 3: Your code here for _alltraps
 */
.globl _alltraps
_alltraps:
#build trapframe
    pushl %ds
    pushl %es
    pushal
#set segment
    movw $(GD_KD), %ax
    movw %ax, %ds
    movw %ax, %es
#call trap
    pushl %esp
    call trap

.globl _retrrap
_retrrap:
    addl $4, %esp 
    popl %es
    popl %ds
    popal
    addl $8, %esp
    iret
