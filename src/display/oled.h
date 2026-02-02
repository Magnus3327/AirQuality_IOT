#ifndef OLED_H
#define OLED_H

#include "pico/stdlib.h"

/** OLED I2C Address */
#define ADDR_OLED 0x3C

/**
 * @brief Initialize the SSD1306 OLED display via I2C.
 */
void oled_init();

/**
 * @brief Clear the entire display buffer.
 */
void oled_clear();

/**
 * @brief Write a single character at a specific position.
 * @param c Character to write
 * @param page OLED page (0-7)
 * @param col Column position (0-127)
 */
void oled_write_char(char c, uint8_t page, uint8_t col);

/**
 * @brief Write a string at a specific position.
 * @param str String to write
 * @param page OLED page (0-7)
 * @param col Column position (0-127)
 */
void oled_write_str(const char* str, uint8_t page, uint8_t col);

#endif