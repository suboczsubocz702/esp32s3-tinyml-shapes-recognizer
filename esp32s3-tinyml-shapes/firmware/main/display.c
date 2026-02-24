#include "display.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   10
#define PIN_NUM_DC   6
#define PIN_NUM_RST  7
#define PIN_NUM_BCKL 3

#define LCD_HOST SPI2_HOST
#define LCD_WIDTH  128
#define LCD_HEIGHT 160

static spi_device_handle_t spi;

static const uint8_t font[][5] = {
	{0x3E,0x51,0x49,0x45,0x3E}, // 0
	{0x00,0x42,0x7F,0x40,0x00}, // 1
	{0x42,0x61,0x51,0x49,0x46}, // 2
	{0x21,0x41,0x45,0x4B,0x31}, // 3
	{0x18,0x14,0x12,0x7F,0x10}, // 4
	{0x27,0x45,0x45,0x45,0x39}, // 5
	{0x3C,0x4A,0x49,0x49,0x30}, // 6
	{0x01,0x71,0x09,0x05,0x03}, // 7
	{0x36,0x49,0x49,0x49,0x36}, // 8
	{0x06,0x49,0x49,0x29,0x1E}, // 9
	{0x7E,0x11,0x11,0x11,0x7E}, // A
	{0x7F,0x49,0x49,0x49,0x36}, // B
	{0x3E,0x41,0x41,0x41,0x22}, // C
	{0x7F,0x41,0x41,0x22,0x1C}, // D
	{0x7F,0x49,0x49,0x49,0x41}, // E
	{0x7F,0x09,0x09,0x09,0x01}, // F
	{0x3E,0x41,0x49,0x49,0x7A}, // G
	{0x7F,0x08,0x08,0x08,0x7F}, // H
	{0x00,0x41,0x7F,0x41,0x00}, // I
	{0x20,0x40,0x41,0x3F,0x01}, // J
	{0x7F,0x08,0x14,0x22,0x41}, // K
	{0x7F,0x40,0x40,0x40,0x40}, // L
	{0x7F,0x02,0x04,0x02,0x7F}, // M
	{0x7F,0x04,0x08,0x10,0x7F}, // N
	{0x3E,0x41,0x41,0x41,0x3E}, // O
	{0x7F,0x09,0x09,0x09,0x06}, // P
	{0x3E,0x41,0x51,0x21,0x5E}, // Q
	{0x7F,0x09,0x19,0x29,0x46}, // R
	{0x46,0x49,0x49,0x49,0x31}, // S
	{0x01,0x01,0x7F,0x01,0x01}, // T
	{0x3F,0x40,0x40,0x40,0x3F}, // U
	{0x1F,0x20,0x40,0x20,0x1F}, // V
	{0x7F,0x20,0x18,0x20,0x7F}, // W
	{0x63,0x14,0x08,0x14,0x63}, // X
	{0x07,0x08,0x70,0x08,0x07}, // Y
	{0x61,0x51,0x49,0x45,0x43}, // Z
	{0x00,0x00,0x5C,0x00,0x00}  // %
};

void lcd_spi_init()
{
	spi_bus_config_t buscfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
	};

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 20 * 1000 * 1000,
		.mode = 0,
		.spics_io_num = PIN_NUM_CS,
		.queue_size = 7,
	};

	spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
	spi_bus_add_device(LCD_HOST, &devcfg, &spi);
}

void lcd_gpio_init()
{
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
	gpio_set_level(PIN_NUM_BCKL, 1);
}

void lcd_reset()
{
	gpio_set_level(PIN_NUM_RST, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(PIN_NUM_RST, 1);
	vTaskDelay(pdMS_TO_TICKS(100));
}

void lcd_cmd(uint8_t cmd)
{
	gpio_set_level(PIN_NUM_DC, 0);
	spi_transaction_t t = {.length = 8, .tx_buffer = &cmd};
	spi_device_transmit(spi, &t);
}

void lcd_data(const uint8_t *data, int len)
{
	gpio_set_level(PIN_NUM_DC, 1);
	spi_transaction_t t = {.length = len * 8, .tx_buffer = data};
	spi_device_transmit(spi, &t);
}

void lcd_init()
{
	lcd_spi_init();
	lcd_gpio_init();
	lcd_reset();

	lcd_cmd(0x11);
	vTaskDelay(pdMS_TO_TICKS(120));

	lcd_cmd(0x3A);
	uint8_t color_mode = 0x05;
	lcd_data(&color_mode, 1);

	lcd_cmd(0x36);
	uint8_t madctl = 0x00;
	lcd_data(&madctl, 1);

	lcd_cmd(0x29);
}

void lcd_set_addr_window(int x0, int y0, int x1, int y1)
{
	uint8_t data[4];

	lcd_cmd(0x2A);
	data[0]=0; data[1]=x0; data[2]=0; data[3]=x1;
	lcd_data(data,4);

	lcd_cmd(0x2B);
	data[0]=0; data[1]=y0; data[2]=0; data[3]=y1;
	lcd_data(data,4);

	lcd_cmd(0x2C);
}

void lcd_draw_pixel(int x, int y, uint16_t color)
{
	if(x<0||x>=LCD_WIDTH||y<0||y>=LCD_HEIGHT) return;

	lcd_set_addr_window(x,y,x,y);
	uint8_t data[2]={color>>8,color&0xFF};
	lcd_data(data,2);
}

void lcd_fill_color(uint16_t color)
{
	lcd_set_addr_window(0,0,LCD_WIDTH-1,LCD_HEIGHT-1);
	uint8_t data[2]={color>>8,color&0xFF};
	gpio_set_level(PIN_NUM_DC,1);

	for(int i=0;i<LCD_WIDTH*LCD_HEIGHT;i++){
		spi_transaction_t t={.length=16,.tx_buffer=data};
		spi_device_transmit(spi,&t);
	}
}

void lcd_draw_rect(int x,int y,int w,int h,uint16_t color)
{
	for(int i=x;i<x+w;i++){
		lcd_draw_pixel(i,y,color);
		lcd_draw_pixel(i,y+h-1,color);
	}
	for(int j=y;j<y+h;j++){
		lcd_draw_pixel(x,j,color);
		lcd_draw_pixel(x+w-1,j,color);
	}
}

void lcd_draw_image_28(uint8_t *img, int offset_x, int offset_y, int scale)
{
	int w = 28 * scale;
	int h = 28 * scale;

	static uint16_t line_buffer[28 * 4];

	lcd_set_addr_window(offset_x,
			offset_y,
			offset_x + w - 1,
			offset_y + h - 1);

	gpio_set_level(PIN_NUM_DC, 1);

	for (int y = 0; y < 28; y++)
	{
		for (int sy = 0; sy < scale; sy++)
		{
			int idx = 0;

			for (int x = 0; x < 28; x++)
			{
				uint16_t color = img[y*28 + x] ? 0xFFFF : 0x0000;

				for (int sx = 0; sx < scale; sx++)
					line_buffer[idx++] = (color >> 8) | (color << 8);
			}

			spi_transaction_t t = {
				.length = w * 16,
				.tx_buffer = line_buffer,
			};

			spi_device_transmit(spi, &t);
		}
	}
}

static const uint8_t* get_char_bitmap(char c)
{
	if(c>='0'&&c<='9') return font[c-'0'];
	if(c>='A'&&c<='Z') return font[10+(c-'A')];
	if(c=='%') return font[36];
	return font[0];
}

static void draw_char(char c,int x,int y,uint16_t color)
{
	const uint8_t *bitmap=get_char_bitmap(c);
	for(int col=0;col<5;col++){
		for(int row=0;row<7;row++){
			if(bitmap[col]&(1<<row))
				lcd_draw_pixel(x+col,y+row,color);
		}
	}
}

void lcd_draw_text(const char *text,int x,int y,uint16_t color)
{
	while(*text){
		draw_char(*text,x,y,color);
		x+=6;
		text++;
	}
}
