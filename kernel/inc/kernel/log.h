/**
 * @file debug/log.h
 */
#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <fmt/_printf.h>
#include <types.h>
#include <stddef.h>
#include <kernel/kprintf.h>

#define LOG_LEVEL_OFF  -1
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

extern struct printf_state __log_printf_state;
extern int                 __log_level;
extern int                 __log_trace;

void log_init(int (*putchar)(int), int level);
void log_setputchar(int (*putchar)(int));
void log_setlevel(int level);

int __log(const char *fmt, ...);
int log(int log_level, const char *fmt, ...);
int trace(const char *fmt, ...);

#define TRACE_ON      do { __log_trace++; } while (0)
#define TRACE_RESTORE do { __log_trace--; } while (0)
#define TRACE_OFF     do { __log_trace = 0; } while (0)

#define INFO(fmt, ...) log(LOG_LEVEL_INFO, "[INFO] "fmt"\n", ##__VA_ARGS__)
#define WARN(fmt, ...) log(LOG_LEVEL_WARN, "[WARN] "fmt"\n", ##__VA_ARGS__)
#define ERROR(fmt, ...) log(LOG_LEVEL_ERROR, "[ERROR] "fmt"\n", ##__VA_ARGS__)

#ifdef KDEBUG
  #define DEBUG_DO(...) \
    do { \
      log(LOG_LEVEL_DEBUG, __VA_ARGS__); \
      kprintf(__VA_ARGS__); \
    } while (0)

  #define DEBUG(fmt, ...) \
    DEBUG_DO("[DEBUG][%s:%d %s()] "fmt"\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

  #define TRACE(fmt, ...) \
    trace("[TRACE] %s("fmt")\n", __func__, ##__VA_ARGS__)
#else
  #define DEBUG(fmt, ...)
  #define TRACE(fmt, ...)
#endif

#endif /* !__DEBUG_LOG_H__ */