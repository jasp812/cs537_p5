#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "mmap.h"
#include "proc.h"
#include "elf.h"

static struct map_mem mapped;

// struct map_mem *mapped allocmmap() {
//     mapped = kalloc();
//     return mapp
// }

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    
    

    struct proc *curproc = myproc();
    
    // int private_bit = flags & MAP_PRIVATE;
    // int shared_bit = flags & MAP_SHARED;
    // int anon_bit = flags & MAP_ANONYMOUS;

    // int fixed_bit = flags & MAP_FIXED;
    // int growsup_bit = flags & MAP_GROWSUP;

    // int read_bit = prot & PROT_READ;
    // int write_bit = prot & PROT_WRITE;

    // cprintf("Value of flags is %d\n", flags);
    // cprintf("Value of MAP_FIXED is %x\n", MAP_FIXED);
    // cprintf("Value of fixed_bit is %x\n", flags & MAP_FIXED);

    // FIXED MAPPING
    if(flags & MAP_FIXED) {
        // Check if address is multiple
        if(!((uint)addr % PGSIZE == 0)){
            cprintf("Address is not page-aligned\n");
            return MAP_FAIL;
        }

        // Check address bounds
        if(!((uint)addr < KERNBASE && (uint)addr >= MMAPBASE)) {
            cprintf("Address is not within bounds\n");
            // seg fault
            kill(curproc->pid);
            return MAP_FAIL;
        }

        // populate struct to reserve memory
        // can also think of this as the 'lazy' part of lazy allocation
        mapped.addr = (uint)addr;
        mapped.length = length;
        mapped.offset = offset;
        mapped.prot = prot;
        mapped.flags = flags;
        mapped.mapped = 1;

        if(mapped.prot & PROT_READ) {
            mapped.prot = mapped.prot | PTE_U;
        }

        if(mapped.prot & PROT_WRITE) {
            mapped.prot = mapped.prot | PTE_W;
        }
        
        // If there is file descriptor, check its valid
        if(!(flags & MAP_ANONYMOUS)) {
            if(fd < 0 || fd >= NOFILE || (mapped.f=myproc()->ofile[fd]) == 0) {
                cprintf("Invalid file descriptor\n");
                return MAP_FAIL;
            }
            mapped.fd = fd;
        }

        // Make sure we aren't exceeding max number of mappings
        if(curproc->num_mappings == MAX_MAPS){
            cprintf("max number of mappings have been reached\n");    
        }

        // Find smallest available index and put the struct at that index
        for(int i = 0; i < MAX_MAPS; i++){
            if(!curproc -> map[i].mapped){
                curproc -> map[i] = mapped;
                curproc->num_mappings++;
                cprintf("mapping placed at index %d\n", i);
                break;
            }
       }
       cprintf("Array updated\n");


    }
    cprintf("Returning %x\n", addr);
    return addr;

}
int munmap(void* addr, size_t length) {
    struct proc *curproc = myproc();

    if(!((uint)addr % PGSIZE)){
        return -1;
    }

    for(int i = 0; i < MAX_MAPS; i++){
        if(curproc -> map[i].addr == (uint)addr){
            curproc -> map[i].addr -= length;
        }
    }



    return 0;
}
