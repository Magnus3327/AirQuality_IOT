#ifndef OLED_H
#define OLED_H

#include "pico/stdlib.h"

#define ADDR_OLED 0x3C

void oled_init();
void oled_clear();
void oled_write_char(char c, uint8_t page, uint8_t col);
void oled_write_str(const char* str, uint8_t page, uint8_t col);

#endif