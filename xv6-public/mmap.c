#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"
#include "mmap.h"

static struct map_mem *mapped;

// struct map_mem *mapped allocmmap() {
//     mapped = kalloc();
//     return mapp
// }

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    
    if(!((uint)addr % PGSIZE)){
            return -1;
    }

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


        mapped->addr = addr;
        mapped->length = length;
        mapped->offset = offset;
        mapped->prot = prot;
        mapped->flags = flags;
        if(fd < 0 || fd >= NOFILE || (mapped->f=myproc()->ofile[fd]) == 0)
            return MAP_FAIL;
        mapped->fd = fd;

        if(curproc->num_mappings == MAX_MAPS){
            cprintf("max number of mappings have been reached\n");    
        }

       for(int i = 0; i < MAX_MAPS; i++){
            if(curproc -> map[i] == 0){
                curproc -> map[i] = mapped;
                curproc->num_mappings++;
                break;
            }
       }


       return addr;

    }


    int munmap(void* addr, size_t length){
        struct proc *curproc = myproc();

        if(!((uint)addr % PGSIZE)){
            return -1;
        }

        for(int i = 0; i < MAX_MAPS; i++){
            if(curproc -> map[i] -> addr == addr){
                curproc -> map[i] -> addr -= length;
            }
        }
    }


}