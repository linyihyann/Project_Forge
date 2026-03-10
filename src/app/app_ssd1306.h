#ifndef APP_SSD1306_H
#define APP_SSD1306_H

#include <stdbool.h>
#include <stdint.h>

// 🌟 抽象化 I2C 寫入介面，App 層不再依賴 HAL
typedef struct
{
    bool (*i2c_tx)(uint8_t dev_addr, const uint8_t* p_data, uint16_t length);
} app_ssd1306_cfg_t;

// 初始化時注入硬體介面
bool app_ssd1306_init(const app_ssd1306_cfg_t* cfg);

// 填滿整個螢幕 (pattern = 0xFF 全亮, 0x00 全暗)
bool app_ssd1306_fill(uint8_t pattern);

#endif /* APP_SSD1306_H */