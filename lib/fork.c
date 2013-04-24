// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    if(!((PTE_W == (PTE_W & vpt[VPN(addr)])) || PTE_COW == (PTE_COW & vpt[VPN(addr)])))
        panic("pgfault: access error!\n");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.

//    envid_t envid = sys_getenvid();
    if (0 != (r = sys_page_alloc(0, PFTEMP, PTE_W | PTE_U | PTE_P)))
        panic("pgfault: %e!\n", r);
    memmove(PFTEMP, (void *)PTE_ADDR(addr), PGSIZE);
    if (0 != (r = sys_page_map(0, PFTEMP, 0, (void *)PTE_ADDR(addr), PTE_W | PTE_U | PTE_P)))
        panic("pgfault: %e!\n", r);
    if (0 != (r = sys_page_unmap(0, PFTEMP)))
        panic("pgfault: %e!\n", r);

    return;
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
// 
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
    pte_t pte = vpt[pn];

    void * va = (void *)(PGSIZE * pn);

    if (0 == pte)
        panic("duppage: error!\n");

    int perm = (pte & 0xfff);

    if (PTE_SHARE & perm) {
        if (0 != (r = sys_page_map(0, va, envid, va, PTE_USER | PTE_SHARE)))
            panic("duppage: %e!", r);
    }
    else if ((PTE_COW == (perm & PTE_COW)) || (PTE_W == (perm & PTE_W))) {
        perm = PTE_U | PTE_COW | PTE_P;
        if (0 != (r = sys_page_map(0, va, envid, va, perm)))
            panic("duppage: %e!", r);

        if (0 != (r = sys_page_map(0, va, 0, va, perm)))
            panic("duppage: %e!", r);
    }
    else {
        if (0 != (r = sys_page_map(0, va, envid, va, perm)))
            panic("duppage: %e!", r);
    }

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "env" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
    // LAB 4: Your code here.

    int r;
    envid_t envid;
    int npte = 0;
    int npde;

    set_pgfault_handler(pgfault);
    envid = sys_exofork();

    if (0 > envid)
        panic("fork: %e!\n", envid);

    if (0 < envid) {
        assert(0 == USTACKTOP % PGSIZE);
        for (npde = PDX(UTEXT); npde <= PDX(USTACKTOP); ++npde) {
            if (vpd[npde] & PTE_P) 
                for (npte = NPDENTRIES * npde; npte < NPDENTRIES * (npde + 1) && npte < VPN(USTACKTOP); ++npte)
                    if (vpt[npte] & PTE_P) 
                        duppage(envid, npte);

            if (VPN(USTACKTOP) == npte)
                break;
        }

        if (0 != (r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W | PTE_P)))
            panic("fork: %e\n", r);

        if (0 != (r = sys_env_set_pgfault_upcall(envid,(void *)env->env_pgfault_upcall)))
            panic("fork: %e\n!", r);

        if (0 != (r = sys_env_set_status(envid, ENV_RUNNABLE)))
            panic("fork: %e!\n", r);

        return envid;
    }

    envid = sys_getenvid();
    env = &envs[ENVX(envid)];

    return 0;
}


// Challenge!
    int
sfork(void)
{
    panic("sfork not implemented");
    return -E_INVAL;
}
