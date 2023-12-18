#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__

#include "st_2_inch/display_config.h"
 
void lcd_write_data_word( uint16_t d );

void lcd_set_cursor( uint16_t x, uint16_t y );
void lcd_set_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end );
void lcd_draw_paint( uint16_t x, uint16_t y, uint16_t color );

void lcd_init( void );
void lcd_set_back_light( uint16_t value );

void lcd_clear( uint16_t color );
void lcd_clear_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color );

#endif // __LCD_DRIVER_H__


