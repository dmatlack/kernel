/**
 * @file debug.h
 *
 * @brief Debugging utilities for the kernel.
 *
 * @author David Matlack
 */
#include <kernel/log.h>
#include <kernel/bochs.h>
#include <assert.h>

#ifndef __DEBUG_H__
#define __DEBUG_H__

void debug_init(void);
int debug_putchar(int c);

#endif /* !__DEBUG_H__ */