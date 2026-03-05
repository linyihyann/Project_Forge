#include "observer.h"

#include <stddef.h>

typedef struct
{
    system_event_id_t event_id;
    event_cb_t callback;
} observer_record_t;

static observer_record_t s_observers[MAX_OBSERVERS];
static uint8_t s_observer_count = 0;

void observer_init(void)
{
    s_observer_count = 0;  // 測試專用：重置觀察者數量，確保每個 Test Case 乾淨
}

int32_t observer_subscribe(system_event_id_t event_id, event_cb_t callback)
{
    if (callback == NULL || s_observer_count >= MAX_OBSERVERS) return -1;
    s_observers[s_observer_count].event_id = event_id;
    s_observers[s_observer_count].callback = callback;
    s_observer_count++;
    return 0;
}

void observer_notify(system_event_id_t event_id, void* context)
{
    for (uint8_t i = 0; i < s_observer_count; i++)
    {
        if (s_observers[i].event_id == event_id)
        {
            s_observers[i].callback(event_id, context);
        }
    }
}