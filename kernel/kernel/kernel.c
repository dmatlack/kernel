/**
 * @file kernel.c
 *
 * Welcome to the main
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/kmalloc.h>
#include <kernel/irq.h>
#include <kernel/timer.h>
#include <kernel/exn.h>
#include <kernel/loader.h>

#include <mm/memory.h>
#include <mm/pages.h>
#include <mm/vm.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

#include <assert.h>
#include <debug.h>
#include <errno.h>

#include <fs/vfs.h>

/**
 * @brief Kernel initialization functions that need to run with interrupts
 * off.
 *
 * You probably don't want to mess with the order of these functions.
 */
void pre_irq_init(void) {
  debug_init();

  pages_init();
  vm_init();
  exn_init();
  irq_init();
  timer_init();
}

/**
 * @brief Kernel initialization functions that can, or should, run with
 * interrupts enabled.
 */
void post_irq_init(void) {
  pci_init();
}

/**
 * @brief This is main logical entry point for the kernel, not to be confused
 * with the actual entry point, _start. Also not to be confused the multiboot
 * entry point mb_entry. Ok so it's not the entry point, but it is an entry
 * point.
 */
void kernel_main() {
  pre_irq_init();
  enable_irqs();
  post_irq_init();
  //run_first_proc();
}
