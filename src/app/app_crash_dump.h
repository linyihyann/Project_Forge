#ifndef APP_CRASH_DUMP_H
#define APP_CRASH_DUMP_H
#include <stdint.h>

#define CRASH_MAGIC_NUMBER 0xDEADBEEFU

typedef struct
{
    uint32_t magic;
    uint32_t crash_timestamp_us;
    char last_log[128];
} crash_dump_t;

extern crash_dump_t g_crash_dump;

void crash_dump_check_and_init(void);
void crash_dump_save_log(const char* msg);

#endif