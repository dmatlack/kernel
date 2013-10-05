/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <dev/vga.h>
#include <x86/exn.h>
#include <x86/pic.h>

void kernel_main() {

  /* initialize the Video Graphics Array so we can start printing */
  if (vga_init()) {
    panic("Unable to initialize the VGA device.\n");
  }

  /* Initialize the exception handlers by installing handlers in the IDT */
  if (x86_exn_install_handlers()) {
    panic("Unable to install x86 exception handlers.\n");
  }

  /* Initialize hardware interrupts by first telling the PIC where in the IDT it 
   * can find its interrupts handlers, and then installing the necessary interrupts 
   * handlers for each device connected to the PIC. */
  pic_init(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET);


  // kernel printing
  kprintf("Hello World!\n");
  // debug printing
  dprintf("Hello World!\n");

  // check exception handling with divide by 0
  kprintf("%d", 1/0);

  while (1) iodelay();
}
