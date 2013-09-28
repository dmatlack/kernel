/**
 * @file _printf.h
 *
 * @brief A putchar-agnostic printf library.
 *
 * @author David Matlack
 */
#ifndef ___PRINTF_H__
#define ___PRINTF_H__

#define	_PRINTF_BUFMAX	128

typedef struct {
  /** @brief internal state used by the print functions */
	char buf[_PRINTF_BUFMAX];
  /** @brief internal state used by the print functions */
	unsigned int index;

  /* @brief the putchar method this version of printf will use */
  int (*putchar)(int c);
} printf_t;

int _vprintf(printf_t *p, const char *fmt, va_list args);

#endif /* ___PRINTF_H__ */

