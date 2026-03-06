#ifndef OBSERVER_H
#define OBSERVER_H
#include <stdint.h>

// cppcheck-suppress misra-c2012-2.5
#define MAX_OBSERVERS 10U

typedef enum
{
    EVENT_SYSTEM_FAULT,
    EVENT_UART_RX_READY
} system_event_id_t;

typedef void (*event_cb_t)(system_event_id_t event_id, void* context);

int32_t observer_subscribe(system_event_id_t event_id, event_cb_t callback);
void observer_notify(system_event_id_t event_id, void* context);
void observer_init(void);

#endif