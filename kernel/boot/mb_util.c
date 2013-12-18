/**
 * @file mb_util.c
 *
 * @brief Utility functions for the multiboot start up code.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>
#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <kernel/kprintf.h>
#include <arch/x86/exn.h>

void mb_exn_handler(struct x86_exn_args *args) {
  struct x86_iret_stack *iret = &args->iret;
  struct x86_pusha_stack *pusha = &args->pusha;
  struct x86_exn *exn = &x86_exceptions[args->vector];

  INFO("-------------------------------------------------------------------");
  INFO("%d %s %s (cause: %s)", exn->vector, exn->mnemonic, exn->description, exn->cause);
  INFO("-------------------------------------------------------------------");
  INFO("eip: 0x%08x", iret->eip);
  INFO("esp: 0x%08x", iret->esp);
  INFO("ebp: 0x%08x", pusha->ebp);
  INFO("");
  INFO("edi: 0x%08x esi: 0x%08x", pusha->edi, pusha->esi);
  INFO("eax: 0x%08x ebx: 0x%08x", pusha->eax, pusha->ebx);
  INFO("ecx: 0x%08x edx: 0x%08x", pusha->ecx, pusha->edx);
  INFO("");
  INFO("cr0: 0x%08x", args->cr0);
  INFO("cr2: 0x%08x", args->cr2);
  INFO("cr3: 0x%08x", args->cr3);
  INFO("cr4: 0x%08x", args->cr4);
  INFO("");
  INFO("ds: 0x%08x", args->ds);
  INFO("es: 0x%08x", args->es);
  INFO("fs: 0x%08x", args->fs);
  INFO("gs: 0x%08x", args->gs);
  INFO("");
  INFO("error: %d", args->error_code);
  INFO("-------------------------------------------------------------------");

  panic("Exception %d during boot. Aborting.", exn->vector);
}


/**
 * @brief Dump the contents of the multiboot_info struct using the given
 * printf function.
 *
 * @param p The printf function to use to do printing.
 * @param mb_info The address of the multiboot info struct.
 */
void mb_dump(printf_f p, struct multiboot_info *mb_info) {
  uint32_t flags = mb_info->flags;
  
  p("struct multiboot_info *: %p\n\n", mb_info);
  p("spec: http://www.gnu.org/software/grub/manual/multiboot/multiboot.txt\n");

#define F(n) ((flags >> (n)) & 0x1)
#define IF_FLAGS(n) \
  p("\nflags[%d] = %d\n", n, F(n)); \
  if (F(n)) 

  IF_FLAGS(0) {
    p("  mem_lower = 0x%08x\n", mb_info->mem_lower * 1024);
    p("  mem_upper = 0x%08x\n", mb_info->mem_upper * 1024);
  }

  IF_FLAGS(1) {
    p("  boot_device = 0x%08x\n", mb_info->boot_device);
  }

  IF_FLAGS(2) {
    p("  cmd_line = %s\n", (char *) mb_info->cmdline);
  }

  IF_FLAGS(3) {
    p("  mods_count = %d\n", mb_info->mods_count);
    p("  mods_addr = 0x%08x\n", mb_info->mods_addr);
  }

  IF_FLAGS(4) {
    p("  aout_sym:\n");
    p("    tabsize = 0x%08x\n", mb_info->u.aout_sym.tabsize); 
    p("    strsize = 0x%08x\n", mb_info->u.aout_sym.strsize); 
    p("    addr = 0x%08x\n", mb_info->u.aout_sym.addr); 
    p("    reserved = 0x%08x\n", mb_info->u.aout_sym.reserved); 
  }

  IF_FLAGS(5) {
    p("  elf_sec:\n");
    p("    num = 0x%08x\n", mb_info->u.elf_sec.num); 
    p("    size = 0x%08x\n", mb_info->u.elf_sec.size); 
    p("    addr = 0x%08x\n", mb_info->u.elf_sec.addr); 
    p("    shndx = 0x%08x\n", mb_info->u.elf_sec.shndx); 
  }

  IF_FLAGS(6) {
    struct multiboot_mmap_entry *e;

    p("  mmap_length = 0x%08x\n", mb_info->mmap_length);
    p("  mmap_addr = 0x%08x\n", mb_info->mmap_addr);

    e = (struct multiboot_mmap_entry *) mb_info->mmap_addr;
    while ((unsigned int) e < (mb_info->mmap_addr + mb_info->mmap_length)) {
      p("    start = 0x%08llx, end = 0x%08llx, len = 0x%08llx, type = %d (%s)\n",
          e->addr, e->addr + e->len, e->len, e->type,
          e->type == MULTIBOOT_MEMORY_AVAILABLE ? "available" : "reserved");
      e++;
    }
  }

  //////
  //TODO eh are these really important?
  //////
  IF_FLAGS(7) {
    p("  TODO\n");
  }

  IF_FLAGS(8) {
    p("  TODO\n");
  }

  IF_FLAGS(9) {
    p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
  }

  IF_FLAGS(10) {
    p("  TODO\n");
  }
  
  IF_FLAGS(11) {
    p("  TODO\n");
  }

#undef FLAGS
}