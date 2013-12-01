/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <assert.h>
#include <debug.h>

#include <kernel/kmalloc.h>
#include <kernel/irq.h>
#include <kernel/timer.h>

#include <mm/vm.h>
#include <mm/physmem.h>

extern void __enable_interrupts(void);

void kernel_main() {

  /*
   * Debug Logging
   */
  SUCCEED_OR_DIE(log_init(dputchar, LOG_LEVEL_DEBUG));

  TRACE_ON;

  /*
   * Hardware Interrupts
   */
  SUCCEED_OR_DIE(irq_init());

  /*
   * Virtual Memory Bootstrap
   */
  SUCCEED_OR_DIE(vm_bootstrap());

  /*
   * Kernel Dynamic Memory Allocation
   */
  SUCCEED_OR_DIE(kmalloc_init());

  /*
   * Physical Memory Manager
   */
  SUCCEED_OR_DIE(pmem_init());

  /*
   * Kernel Timer
   */
  SUCCEED_OR_DIE(timer_init(100));

  kprintf(
    "\n"
    "  All that is gold does not glitter,\n"
    "  Not all those who wander are lost;\n"
    "  The old that is strong does not wither,\n"
    "  Deep roots are not reached by the frost.\n"
    "\n"
    "  From the ashes a fire shall be woken,\n"
    "  A light from the shadows shall spring;\n"
    "  Renewed shall be blade that was broken,\n"
    "  The crownless again shall be king\n"
    "\n"
    "          J.R.R. Tolkien\n"
    "\n");

  generate_irq(0);
  //__enable_interrupts();

  while (1) continue;
}
