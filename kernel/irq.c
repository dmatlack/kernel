/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 * @author David Matlack
 */
#include <kernel/irq.h>

#include <dev/vga.h>

#include <kernel.h>
#include <debug.h>
#include <assert.h>
#include <list.h>

struct irq_state {
  irq_handler_list_t handlers;
  int count;
  int in_irq;
};

int __max_irqs;
struct irq_state *__irqs;

#ifdef ARCH_X86
extern struct machine_irq_interface x86_irq_interface;
static struct machine_irq_interface *__machine_irq_system = &x86_irq_interface;
#endif

int irq_init(void) {
  struct machine_irq_info irq_info;
  int i;

  TRACE("");

  if (0 != __machine_irq_system->init(&irq_info)) {
    return -1;
  }

  __max_irqs = irq_info.max_irqs;

  __irqs = kmalloc(sizeof(struct irq_state) * __max_irqs);
  if (NULL == __irqs) {
    kprintf("NULL == __irqs\n");
    return -1;
  }
  
  for (i = 0; i < __max_irqs; i++) {
    struct irq_state *irq = __irqs + i;
    list_init(&irq->handlers);
    irq->count = 0;
    irq->in_irq = 0;
  }

  return 0;
}

void generate_irq(int irq) {
  __machine_irq_system->generate_irq(irq);
}

void acknowledge_irq(int irq) {
  __machine_irq_system->acknowledge_irq(irq);
}


void handle_irq(int irq) {
  struct irq_context context;
  struct irq_state *state;
  struct irq_handler *handler;

  state = &__irqs[irq];

  //FIXME atomic test and set
  ASSERT(0 == state->in_irq);
  state->in_irq++;
  state->count++;

  context.irq = irq;

  list_foreach(handler, &state->handlers, link) {
    if (NULL != handler->top_handler) handler->top_handler(&context);
  }

  state->in_irq--;
  acknowledge_irq(irq);

  {
    int row,col;
    vga_get_cursor(&row, &col);
    vga_set_cursor(24, 0);
    kprintf("Timer: %8d Keyboard: %8d", __irqs[0].count, __irqs[1].count);
    vga_set_cursor(row, col);
  }
}

void register_irq(int irq, struct irq_handler *new_handler) {
  ASSERT(irq >= 0 && irq <= 15);

  list_elem_init(new_handler, link);

  //TODO disable_interrupts

  list_insert_tail(&__irqs[irq].handlers, new_handler, link);

  //TODO restore_interrupts
}
