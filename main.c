#include <stdint.h>

#include "app_main.h"

uint8_t DOOM_ARRAY[6000];

int main(void)
{
    // 初始化硬體
    app_main_init();

    // 進入無窮迴圈
    while (1)
    {
        app_main_task();
    }

    return 0;
}