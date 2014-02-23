/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory Manager
 *
 * @author David Matlack
 */
#ifndef __MM_VM_H__
#define __MM_VM_H__

#include <string.h>
#include <stddef.h>
#include <list.h>

#define VM_R (1 << 0) // read
#define VM_W (1 << 1) // write
#define VM_X (1 << 2) // execute
#define VM_U (1 << 3) // user
#define VM_S (1 << 4) // supervisor
#define VM_G (1 << 5) // global

struct vm_region {
  size_t address;
  size_t size;
  int flags;

  /*
   * When performing copy-on-write and zero-fill-on-demand, regions of
   * memory will be shared with other processes until they are accessed.
   * At that point we need to fulfill the page request with a physical
   * page. These pages are reserved by this region. This counter keeps
   * track of how many pages have been reserved but not allocated. Thus
   * if a process doesn't end up accessing the page, we don't forget to
   * free it.
   */
  unsigned int unfulfilled_ppages;

  list_link(struct vm_region) link;
};

list_typedef(struct vm_region) vm_region_list_t;

struct vm_space {
  void *object;

  struct vm_region *kernel_region;
  struct vm_region *stack_region;
  struct vm_region *heap_region;
  vm_region_list_t other_regions;
};

extern struct vm_space boot_vm_space;

void vm_init(void);
int  vm_space_init(struct vm_space *space);
void *__vm_space_switch(void *space_object);
int  vm_map(struct vm_space *space, size_t address, size_t size, int flags);
int  __vm_map(struct vm_space *space, size_t address, size_t size, size_t *ppages, int flags);
void vm_unmap(struct vm_space *space, size_t address, size_t size);

#endif /* !__MM_VM_H__ */
