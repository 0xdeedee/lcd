#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "lcd_driver.h"
#include "gui_paint.h"

#include "st_2_inch/fonts.h"

#include "st_2_inch/font24.h"


extern spi_device_handle_t             __spi;

typedef enum {
    LCD_TYPE_ILI = 1,
    LCD_TYPE_ST,
    LCD_TYPE_MAX,
} type_lcd_t;

#define LCD_CTRL_BITMASK	( ( 1ULL << PIN_NUM_DC ) | ( 1ULL << PIN_NUM_RST ) | ( 1ULL << PIN_NUM_BCKL ) )
#define LINE_VALUE_SIZE		( 12 )			// 11 symbols + zero terminator

static __s_font_t	*__font = &__font_24;


static void draw_table( void )
{

	paint_new_image( LCD_WIDTH, LCD_HEIGHT, ROTATE_270, ~WHITE );
	paint_clear(~WHITE);

	paint_draw_line( 1,	0,	1,	320,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 40,	0,	40,	320,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 239,	0,	239,	320,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	2,	238,	2,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	28,	238,	28,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	56,	238,	56,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	83,	238,	83,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	110,	238,	110,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	137,	238,	137,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	164,	238,	164,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	191,	238,	191,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	218,	238,	218,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	245,	238,	245,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	272,	238,	272,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	295,	238,	295,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );
	paint_draw_line( 0,	319,	238,	319,	~BLACK,	DOT_PIXEL_1X1,	LINE_STYLE_SOLID );


	paint_draw_string( 3,	3,	"CT",	__font,		~WHITE,	~BLACK );
	paint_draw_string( 3,	31,	"PT",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	57,	"I",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	84,	"U",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	111,	"N",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	138,	"T",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	165,	"P1",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	192,	"P2",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	219,	"E",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	246,	"L",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	273,	"",	&__font_24,	~WHITE,	~BLACK );
	paint_draw_string( 3,	296,	"",	&__font_24,	~WHITE,	~BLACK );

}

static void update_line_1( unsigned int coil_a_temp, unsigned int coil_b_temp, unsigned int coil_c_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", coil_a_temp, coil_b_temp, coil_c_temp );
	paint_draw_string( 50, 3, line, __font, ~WHITE, ~BLACK );
}

static void update_line_2( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 31, line, __font, ~WHITE, ~BLACK );
}

static void update_line_3( float input_current )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%04f", input_current );
	paint_draw_string( 50, 57, line, __font, ~WHITE, ~BLACK );
}

static void update_line_4( float input_voltage )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%04f", input_voltage );
	paint_draw_string( 50, 84, line, __font, ~WHITE, ~BLACK );
}

static void update_line_5( float rpm )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%04f", rpm );
	paint_draw_string( 50, 111, line, __font, ~WHITE, ~BLACK );
}

static void update_line_6( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 138, line, __font, ~WHITE, ~BLACK );
}

static void update_line_7( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 165, line, __font, ~WHITE, ~BLACK );
}

static void update_line_8( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 192, line, __font, ~WHITE, ~BLACK );
}

static void update_line_9( float p_in, float p_out )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%0.4f", ( p_out / p_in ) );
	paint_draw_string( 50, 216, line, __font, ~WHITE, ~BLACK );
}

static void update_line_10( float p_in, float p_out )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%04f", ( p_in - p_out ) );
	paint_draw_string( 50, 246, line, __font, ~WHITE, ~BLACK );
}

static void update_line_11( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 273, line, __font, ~WHITE, ~BLACK );
}

static void update_line_12( unsigned int t_temp, unsigned int pcb_temp, unsigned int drv_temp )
{
	char		line[LINE_VALUE_SIZE];

	memset( line, 0, sizeof ( line ) );
	snprintf( line, ( LINE_VALUE_SIZE ), "%03d %03d %03d", t_temp, pcb_temp, drv_temp );
	paint_draw_string( 50, 296, line, __font, ~WHITE, ~BLACK );
}

void app_main( void )
{
	lcd_init_device();
	lcd_init();

	draw_table();






	update_line_1( 111, 222, 333 );
	update_line_2( 11, 22, 33 );
	update_line_3( 0.8954 );
	update_line_4( 24.98454 );
	update_line_5( 111.5673 );
	update_line_6( 11, 22, 33 );
	update_line_7( 11, 22, 33 );
	update_line_8( 11, 22, 33 );
	update_line_9( 2.8493, 1.974 );
	update_line_10( 2.8493, 1.974 );
	update_line_11( 11, 22, 33 );
	update_line_12( 11, 22, 33 );


	while ( 1 )
	{
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}

	return;
}




