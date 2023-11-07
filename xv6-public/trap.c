#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "mmap.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

void*
pgfltpfhpgflthndlrintr() 
{
  struct proc *p = myproc();
  uint fault_addr = rcr2();
  
  cprintf("Entering for loop\n");
  for(int i = 0; i < p->num_mappings; i++) {

    // struct map_mem maps[32] = p->map;
    uint maxaddr = PGROUNDUP(((uint)p->map[i].addr) + p->map[i].length); 

    cprintf("fault_addr: %x, process mapp addr: %x, maxaddr: %x\n", fault_addr, p->map[i].addr, maxaddr);

    // Check whether the virtual address being accessed is within bounds
    if(fault_addr < p->map[i].addr || fault_addr > maxaddr) {
      cprintf("Virtual address out of bounds\n");
      cprintf("Segmentation fault. wahahaha skill issue\n");
      p->killed = 1;
      return MAP_FAIL;
    }

    pde_t *pte;

    // pte = walkpgdir(p->pgdir, fault_addr, maps[i]->length);

    // Check that the address of the pte associated with the virtual addresss is valid
    if((pte = walkpgdir(p->pgdir, (void*)fault_addr, p->map[i].length)) == 0) {
      cprintf("PTE not valid\n");
      cprintf("Segmentation fault. wahahaha skill issue\n");
      p->killed = 1;
      return MAP_FAIL;
    }

    // Check physical address of the pte
    if(PTE_ADDR(&pte) == 0) {
      cprintf("Physical address of pte not valid\n");
      cprintf("Segmentation fault. wahahaha skill issue\n");
      p->killed = 1;
      return MAP_FAIL;
    }

    cprintf("Checking flags...\n");

    // ANON MAPPING
    if(p->map[i].flags & MAP_ANONYMOUS) {
      cprintf("Entering ANON MAPPING\n");
      int j;
      int length = p->map[i].length;
      uint addr = p->map[i].addr;

      // For each page...
      for(j = 0; j < length; j += PGSIZE) {
        char *page = kalloc();

        // Check return value of kalloc()
        if (!page) {
          return MAP_FAIL;
        }

        // Zero out page to prep for mapping
        memset((void*)page, 0, length);

        int ret = mappages(p->pgdir, (void*)(fault_addr + j), PGSIZE, V2P(page), p->map[i].prot);
        
        // Check if mappages failed, if it did: deallocate the kalloc'ed memory and free pointer
        if(ret != 0) {
          deallocuvm(p->pgdir, addr - PGSIZE, addr);
          kfree(page);
          return MAP_FAIL;
        }
      }     
    } else { // FILE-BACKED MAPPING
  
    }




    
  }

  return MAP_SUCCESS;
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;
  case T_PGFLT:
    pgfltpfhpgflthndlrintr();
    break;


  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
