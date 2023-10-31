#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "mmap.h"

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    struct proc *curproc = myproc();
    
    int private_bit = flags & MAP_PRIVATE;
    int shared_bit = flags & MAP_SHARED;
    int anon_bit = flags & MAP_ANONYMOUS;
    int fixed_bit = flags & MAP_FIXED;
    int growsup_bit = flags & MAP_GROWSUP;

    int read_bit = prot & PROT_READ;
    int write_bit = prot & PROT_WRITE;

    

    if(fixed_bit == MAP_FIXED) {
        // Check address bounds
        if(!((uint)addr < KERNBASE && (uint)addr >= MMAPBASE)) {
            // seg fault
            kill(curproc->pid);
            cprintf("Segmentation fault!\n");
            return;
        }

        
    }




}