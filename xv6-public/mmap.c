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
    uint private_mask 
}