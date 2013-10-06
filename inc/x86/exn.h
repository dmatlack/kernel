/**
 * @file exn.h
 *
 * @brief Defines for x86 Exceptions.
 *
 * Faults: An exception that can generally be corrected and the program can
 *    be restarted with no loss of continuity. When the fault occurs, the
 *    machine resets its state to the state PRIOR to the instruction that
 *    caused the fault. Thus the "return address" of the exception handler 
 *    points to the instruction that caused the fault.
 *
 * Traps: An exception that is reported immediately following the execution 
 *    of the trapping instruction. The return address of the trap handler
 *    points to the instruction following the trapping instruction.
 *
 * Aborts: An abort is an exception that does not always report the location
 *    of the instruction causing the exception and does not allow the 
 *    program to continue execution after the handler completes.
 *
 * @author David Matlack
 */
#ifndef __X86_EXN_H__
#define __X86_EXN_H__

#include <stdint.h>
#include <types.h>

#include <x86/reg.h>
#include <x86/idt.h>

#define X86_TRAP  0x1
#define X86_FAULT 0x2
#define X86_ABORT 0x4
#define X86_IS_TRAP(type)   (type & X86_TRAP)
#define X86_IS_FAULT(type)  (type & X86_FAULT)
#define X86_IS_ABORT(type)  (type & X86_ABORT)

struct x86_exn {
  uint8_t vector;
  char    mnemonic[5];
  char    description[128];
  uint8_t type;
  char    cause[128];
  bool    has_error_code;
  void (*handler)(void);
};

#define X86_NUM_EXCEPTIONS 20
extern struct x86_exn x86_exceptions[X86_NUM_EXCEPTIONS];

// WARNING
//  If you change this macro, you must also change the macro in
//  x86/exn_wrappers.S !!!!!!!!!!
#define EXN_HANDLER_NAME(vector)        x86_exn_handle_##vector

#define EXN_HANDLER_PROTOTYPE(vector)   void EXN_HANDLER_NAME(vector)(void)

/*
 * Generate the exception handler prototypes
 */
EXN_HANDLER_PROTOTYPE(0);
EXN_HANDLER_PROTOTYPE(1);
EXN_HANDLER_PROTOTYPE(2);
EXN_HANDLER_PROTOTYPE(3);
EXN_HANDLER_PROTOTYPE(4);
EXN_HANDLER_PROTOTYPE(5);
EXN_HANDLER_PROTOTYPE(6);
EXN_HANDLER_PROTOTYPE(7);
EXN_HANDLER_PROTOTYPE(8);
EXN_HANDLER_PROTOTYPE(9);
EXN_HANDLER_PROTOTYPE(10);
EXN_HANDLER_PROTOTYPE(11);
EXN_HANDLER_PROTOTYPE(12);
EXN_HANDLER_PROTOTYPE(13);
EXN_HANDLER_PROTOTYPE(14);
EXN_HANDLER_PROTOTYPE(15);
EXN_HANDLER_PROTOTYPE(16);
EXN_HANDLER_PROTOTYPE(17);
EXN_HANDLER_PROTOTYPE(18);
EXN_HANDLER_PROTOTYPE(19);

struct __attribute__((__packed__)) x86_exn_args {
  /* exception vector */
  uint32_t vector;
  /* control registers */
  uint32_t cr2;
  uint32_t cr3;
  /* segment selectors */
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
  /* pusha registers */
  struct x86_pusha_stack pusha;
  /* error code for exception  */
  uint32_t error_code;
  /* machine generated state information */
  struct x86_iret_stack iret;
};

/**
 * @brief Initialize the x86 exception handling by installing base 
 * handlers into the IDT.
 */
int x86_exn_init(void (*handler)(struct x86_exn_args *exn));

/**
 * @brief The handler for all exceptions. This is NOT the handler installed
 * in the IDT for exceptions. Instead, all of those handlers call into this
 * handler, passing in state such as the pusha stack, and the exception vector.
 *
 * DO NOT CALL THIS. This is called by x86/exn_wrappers.S.
 */
void x86_exn_handle_all(struct x86_exn_args args);

#endif /* __X86_EXN_H__ */
