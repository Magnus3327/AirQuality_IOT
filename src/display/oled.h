#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <stdbool.h>
#include <stdint.h>

#include "app_config.h"

void oled_init(void);
void oled_clear(void);
void oled_draw_text(uint8_t x, uint8_t y, const char *text);
void oled_flush(void);
void oled_show_basic(const telem_t *t);

#endif /* DISPLAY_OLED_H */
