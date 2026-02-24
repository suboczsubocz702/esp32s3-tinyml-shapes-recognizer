#include "image_generator.h"
#include "esp_random.h"

void clear_image(uint8_t *img)
{
	for (int i = 0; i < IMG_SIZE * IMG_SIZE; i++)
		img[i] = 0;
}

static int noise_min = 5;
static int noise_max = 20;

void set_noise_range(int min_percent, int max_percent)
{
	noise_min = min_percent;
	noise_max = max_percent;
}

static int apply_noise(uint8_t *img)
{
	int noise_percent = 5 + (esp_random() % 16);

	for (int i = 0; i < IMG_SIZE * IMG_SIZE; i++)
	{
		if ((esp_random() % 100) < noise_percent)
			img[i] = !img[i];
	}

	return noise_percent;
}

int generate_square(uint8_t *img)
{
	clear_image(img);

	int size = 6 + (esp_random() % 8);

	int start_x = esp_random() % (IMG_SIZE - size);
	int start_y = esp_random() % (IMG_SIZE - size);

	for (int y = start_y; y < start_y + size; y++)
		for (int x = start_x; x < start_x + size; x++)
			img[y * IMG_SIZE + x] = 1;

	return apply_noise(img);
}

int generate_triangle(uint8_t *img)
{
	clear_image(img);

	int height = 8 + (esp_random() % 10);
	int base_width = height;

	int start_x = esp_random() % (IMG_SIZE - base_width);
	int start_y = esp_random() % (IMG_SIZE - height);

	for (int y = 0; y < height; y++)
	{
		int current_width = (y * base_width) / height;
		int offset = (base_width - current_width) / 2;

		for (int x = 0; x < current_width; x++)
		{
			int px = start_x + offset + x;
			int py = start_y + y;

			if (px >= 0 && px < IMG_SIZE && py >= 0 && py < IMG_SIZE)
				img[py * IMG_SIZE + px] = 1;
		}
	}

	return apply_noise(img);
}

int generate_circle(uint8_t *img)
{
	clear_image(img);

	int r = 6 + (esp_random() % 6);

	int cx = r + (esp_random() % (IMG_SIZE - 2 * r));
	int cy = r + (esp_random() % (IMG_SIZE - 2 * r));

	for (int y = 0; y < IMG_SIZE; y++)
	{
		for (int x = 0; x < IMG_SIZE; x++)
		{
			int dx = x - cx;
			int dy = y - cy;

			if (dx * dx + dy * dy <= r * r)
				img[y * IMG_SIZE + x] = 1;
		}
	}

	return apply_noise(img);
}
