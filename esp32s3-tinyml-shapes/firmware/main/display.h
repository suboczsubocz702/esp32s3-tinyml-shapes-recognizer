#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	void lcd_init(void);
	void lcd_fill_color(uint16_t color);
	void lcd_draw_pixel(int x, int y, uint16_t color);
	void lcd_draw_image_28(uint8_t *img, int offset_x, int offset_y, int scale);
	void lcd_draw_rect(int x, int y, int w, int h, uint16_t color);
	void lcd_draw_text(const char *text, int x, int y, uint16_t color);

#ifdef __cplusplus
}
#endif
