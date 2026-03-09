#ifndef APP_SSD1306_H
#define APP_SSD1306_H

#include <stdbool.h>
#include <stdint.h>

/* 初始化 SSD1306 */
bool app_ssd1306_init(void);

/* 填滿整個螢幕 (pattern = 0xFF 全亮, 0x00 全暗) */
bool app_ssd1306_fill(uint8_t pattern);

#endif /* APP_SSD1306_H */