/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <x86/io.h>
#include <dev/vga.h>
#include <stdio.h>

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

void delay(void) {
  int i;
  for (i = 0; i < 500000; i++) {
    iodelay();
  }
}
 
void kernel_main() {
  int i;

  vga_init();

  printf("%s %s %s\n", "Things seem to be working", "!", "=)");

  while (1) delay();
}
