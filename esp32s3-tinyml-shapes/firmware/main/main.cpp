#include "display.h"
#include "image_generator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include <stdio.h>

#include "model.cc"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define TENSOR_ARENA_SIZE (60 * 1024)
static uint8_t tensor_arena[TENSOR_ARENA_SIZE];

#define CLASS_TRIANGLE 0
#define CLASS_SQUARE   1
#define CLASS_CIRCLE   2

static int64_t total_runtime_us = 0;
static int64_t total_infer_us = 0;

uint8_t img[28 * 28];

void send_sample(uint8_t *img, int class_id, int noise_level)
{
	printf("%d,%d,", class_id, noise_level);

	for (int i = 0; i < 784; i++)
	{
		printf("%d", img[i]);
		if (i < 783) printf(",");
	}

	printf("\n");
}

extern "C" void app_main(void)
{
	lcd_init();
	lcd_fill_color(0x0000);

	const tflite::Model* model = tflite::GetModel(g_model);

	static tflite::MicroMutableOpResolver<8> resolver;
	resolver.AddConv2D();
	resolver.AddMaxPool2D();
	resolver.AddReshape();
	resolver.AddFullyConnected();
	resolver.AddSoftmax();
	resolver.AddRelu();
	resolver.AddMean();

	static tflite::MicroInterpreter interpreter(
			model,
			resolver,
			tensor_arena,
			TENSOR_ARENA_SIZE
			);

	if (interpreter.AllocateTensors() != kTfLiteOk)
		return;

	TfLiteTensor* input = interpreter.input(0);
	TfLiteTensor* output = interpreter.output(0);

	size_t arena_used = interpreter.arena_used_bytes();

	set_noise_range(5, 20);

	while (1)
	{
		int64_t loop_start = esp_timer_get_time();

		int class_id = esp_random() % 3;
		int noise_level = 0;

		if (class_id == CLASS_TRIANGLE)
			noise_level = generate_triangle(img);
		else if (class_id == CLASS_SQUARE)
			noise_level = generate_square(img);
		else
			noise_level = generate_circle(img);

		lcd_draw_rect(0, 40, 128, 120, 0x0000);
		lcd_draw_image_28(img, 20, 50, 4);

		for (int i = 0; i < 784; i++)
			input->data.int8[i] = img[i] ? 127 : -128;

		int64_t infer_start = esp_timer_get_time();

		if (interpreter.Invoke() != kTfLiteOk)
			continue;

		int64_t infer_end = esp_timer_get_time();

		int8_t* scores = output->data.int8;

		int predicted = 0;
		int8_t max_score = scores[0];

		for (int i = 1; i < 3; i++)
		{
			if (scores[i] > max_score)
			{
				max_score = scores[i];
				predicted = i;
			}
		}

		float scale = output->params.scale;
		int zero_point = output->params.zero_point;
		float confidence = (max_score - zero_point) * scale;
		if (confidence < 0) confidence = 0;
		if (confidence > 1) confidence = 1;

		int percent = (int)(confidence * 100.0f);

		uint16_t frame_color = (predicted == class_id) ? 0x07E0 : 0xF800;
		lcd_draw_rect(18, 48, 28 * 4 + 4, 28 * 4 + 4, frame_color);

		char buf[64];

		lcd_draw_rect(0, 150, 128, 20, 0x0000);

		if (predicted == class_id)
			lcd_draw_text("OK", 20, 150, 0x07E0);
		else
			lcd_draw_text("WRONG", 20, 150, 0xF800);

		snprintf(buf, sizeof(buf), "%d%%", percent);
		lcd_draw_text(buf, 80, 150, 0xFFFF);

		size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
		size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
		size_t used_heap = total_heap - free_heap;
		size_t min_heap = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);

		int64_t loop_end = esp_timer_get_time();

		total_runtime_us += (loop_end - loop_start);
		total_infer_us += (infer_end - infer_start);

		float infer_ms = (infer_end - infer_start) / 1000.0f;

		float cpu_percent = 0.0f;
		if (total_runtime_us > 0)
			cpu_percent = ((float)total_infer_us / (float)total_runtime_us) * 100.0f;

		lcd_draw_rect(0, 0, 128, 40, 0x0000);

		snprintf(buf, sizeof(buf), "RAM %dKB", (int)(used_heap / 1024));
		lcd_draw_text(buf, 4, 4, 0xFFFF);

		snprintf(buf, sizeof(buf), "Min %dKB", (int)(min_heap / 1024));
		lcd_draw_text(buf, 4, 12, 0xF800);

		snprintf(buf, sizeof(buf), "Arena %dKB", (int)(arena_used / 1024));
		lcd_draw_text(buf, 4, 20, 0x07E0);

		snprintf(buf, sizeof(buf), "CPU %d%% %.1fms",
				(int)cpu_percent,
				infer_ms);
		lcd_draw_text(buf, 4, 28, 0xFFFF);

		printf("True:%d Noise:%d Pred:%d Conf:%d%% CPU:%.1f%%\n",
				class_id,
				noise_level,
				predicted,
				percent,
				cpu_percent);

		send_sample(img, class_id, noise_level);

		vTaskDelay(pdMS_TO_TICKS(200));
	}
}
