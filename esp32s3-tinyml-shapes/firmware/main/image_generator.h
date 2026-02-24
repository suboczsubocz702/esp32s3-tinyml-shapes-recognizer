#pragma once
#include <stdint.h>

#define IMG_SIZE 28

#ifdef __cplusplus
extern "C" {
#endif

	void clear_image(uint8_t *img);
	void set_noise_range(int min_percent, int max_percent);

	int generate_square(uint8_t *img);
	int generate_triangle(uint8_t *img);
	int generate_circle(uint8_t *img);

#ifdef __cplusplus
}
#endif
