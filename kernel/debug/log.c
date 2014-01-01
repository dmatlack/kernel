/**
 * @file debug/log.c
 *
 * @author David Matlack
 */
#include <debug.h>

#include <stddef.h>
#include <types.h>
#include <stdarg.h>
#include <errno.h>

#include <kernel/kprintf.h>

struct printf_state __log_printf_state;
int                 __log_level;
int                 __log_trace;

/**
 * @brief Initialize the logging system.
 *
 * @param putchar The putchar method to use for printing.
 *
 * @param level   The verbosity level of logging to use. This can always
 *                be changed dynamically with log_setlevel().
 *
 * @return 0
 */
int log_init(int (*putchar)(int), int level) {
  __log_printf_state.putchar = putchar;
  __log_level = level;
  __log_trace = 1;
  return 0;
}

/**
 * @brief Set the putchar method to be used for future logging.
 */
void log_setputchar(int (*putchar)(int)) {
  __log_printf_state.putchar = putchar;
}

/**
 * @brief Set the verbosity level of future logging.
 */
void log_setlevel(int level) {
  __log_level = level;
}

/**
 * @brief Log a formatted string.
 *
 * @param log_level The minimum level needed to print this string.
 * @param fmt       The format string.
 * @param ...       The format arguments.
 */
int log(int log_level, const char *fmt, ...) {
  va_list args;
  int err;

  va_start(args, fmt);

  if (log_level <= __log_level) {
    err = _vprintf(&__log_printf_state, fmt, args);
  }

  va_end(args);

  return err;
}

/**
 * @brief Print a formatted trace string if tracing is enabled.
 */
int trace(const char *fmt, ...) {
  va_list args;
  int err;

  va_start(args, fmt);

  if (__log_trace> 0) {
    err = _vprintf(&__log_printf_state, fmt, args);
  }

  va_end(args);

  return err;
}
