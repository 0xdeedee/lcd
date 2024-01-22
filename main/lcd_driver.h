#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"



#include "st_2_inch/display_config.h"

typedef struct {
	uint8_t			cmd;
	uint8_t			data[16];
	uint8_t			databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void lcd_set_cursor( uint16_t x, uint16_t y );
void lcd_set_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end );
void lcd_draw_paint( uint16_t x, uint16_t y, uint16_t color );

void lcd_init();
void lcd_init_device();
void lcd_set_back_light( uint16_t value );

void lcd_clear( uint16_t color );
void lcd_clear_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color );
void lcd_clear( uint16_t color );
#endif // __LCD_DRIVER_H__


