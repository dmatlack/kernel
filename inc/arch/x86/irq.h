/**
 * @file x86/irq.h
 *
 * @author David Matlack
 */
#ifndef __X86_IRQ_H__
#define __X86_IRQ_H__

#include <arch/x86/idt.h>
#include <list.h>

#define MAX_IRQS 16
/*
 *
 * x86 Hardware Interrupts
 * 
 * 8259A Input pin  Interrupt Number  Description
 * ------------------------------------------------------------------
 * IRQ0             0x08              Timer
 * IRQ1             0x09              Keyboard
 * IRQ2             0x0A              Cascade for 8259A Slave controller
 * IRQ3             0x0B              Serial port 2
 * IRQ4             0x0C              Serial port 1
 * IRQ5             0x0D              AT systems: Parallel Port 2. PS/2 systems: reserved
 * IRQ6             0x0E              Diskette drive
 * IRQ7             0x0F              Parallel Port 1
 * IRQ8/IRQ0        0x70              CMOS Real time clock
 * IRQ9/IRQ1        0x71              CGA vertical retrace
 * IRQ10/IRQ2       0x72              Reserved
 * IRQ11/IRQ3       0x73              Reserved
 * IRQ12/IRQ4       0x74              AT systems: reserved. PS/2: auxiliary device
 * IRQ13/IRQ5       0x75              FPU
 * IRQ14/IRQ6       0x76              Hard disk controller
 * IRQ15/IRQ7       0x77              Reserved
 *
 */
#define IRQ_TIMER         0x0
#define IRQ_KEYBOARD      0x1

struct irq_context {
  int irq;
};

typedef void (*irq_handler_f)(struct irq_context *context);

struct irq_handler {
  /**
   * @brief The top_handler, if registered, is executed in interrupt context,
   * while interrupts are disabled. It should be short and sweet.
   */
  irq_handler_f top_handler;
  /**
   * @brief The bottom_handler, if registered, is executed some time after
   * the interrupt has been acknowledged. It runs with interrupts enabled.
   */
  irq_handler_f bottom_handler;

  list_link(struct irq_handler) link;
};

list_typedef(struct irq_handler) irq_handler_list_t;

int irq_init(void);
void register_irq(int irq, struct irq_handler *handler);

void __int(uint8_t n);
void generate_irq(int irq);
void handler_irq(int irq);

void __enable_interrupts(void);
void __disable_interrupts(void);

#endif /* !__X86_IRQ_H__ */
