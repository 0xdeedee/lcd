#ifndef __GUI_PAINT_H__
#define __GUI_PAINT_H__

#include "lcd_driver.h"
#include "st_2_inch/fonts.h"



/**
 * Image attributes
**/
typedef struct {
	uint8_t		*image;
	uint16_t	width;
	uint16_t	height;
	uint16_t	width_memory;
	uint16_t	height_memory;
	uint16_t	color;
	uint16_t	rotate;
	uint16_t	mirror;
	uint16_t	width_byte;
	uint16_t	height_byte;
} paint_t;
extern volatile paint_t		__paint;

/**
 * Display rotate
**/
#define ROTATE_0			( 0 )
#define ROTATE_90			( 90 )
#define ROTATE_180			( 180 )
#define ROTATE_270			( 270 )

/**
 * Display Flip
**/
typedef enum {
	MIRROR_NONE		= 0x00,
	MIRROR_HORIZONTAL	= 0x01,
	MIRROR_VERTICAL		= 0x02,
	MIRROR_ORIGIN		= 0x03,
} mirror_image_t;
#define MIRROR_IMAGE_DFT MIRROR_NONE

/**
 * image color
**/

#define WHITE				( 0xFFFF )
#define BLACK				( 0x0000 )   
#define BLUE				( 0x001F )
#define BRED				( 0xF81F )
#define GRED				( 0xFFE0 )
#define GBLUE				( 0x07FF )
#define RED				( 0xF800 )
#define MAGENTA				( 0xF81F )
#define GREEN				( 0x07E0 )
#define CYAN				( 0x7FFF )
#define YELLOW				( 0xFFE0 )
#define BROWN				( 0xBC40 )
#define BRRED				( 0xFC07 )
#define GRAY				( 0x8430 )
#define DARKBLUE			( 0x01CF )
#define LIGHTBLUE			( 0x7D7C ) 
#define GRAYBLUE			( 0x5458 )
#define LIGHTGREEN			( 0x841F )
#define LGRAY				( 0xC618 )
#define LGRAYBLUE			( 0xA651 )
#define LBBLUE				( 0x2B12 )


#define IMAGE_BACKGROUND		( WHITE )
#define FONT_FOREGROUND			( BLACK )
#define FONT_BACKGROUND			( WHITE )

/**
 * The size of the point
**/
typedef enum {
	DOT_PIXEL_1X1  = 1,   // 1 x 1
	DOT_PIXEL_2X2  ,    // 2 X 2
	DOT_PIXEL_3X3  ,    // 3 X 3
	DOT_PIXEL_4X4  ,    // 4 X 4
	DOT_PIXEL_5X5  ,    // 5 X 5
	DOT_PIXEL_6X6  ,    // 6 X 6
	DOT_PIXEL_7X7  ,    // 7 X 7
	DOT_PIXEL_8X8  ,    // 8 X 8
} dot_pixel_t;
#define DOT_PIXEL_DFT			( DOT_PIXEL_1X1 )  //Default dot pilex

/**
 * Point size fill style
**/
typedef enum {
	DOT_FILL_AROUND  = 1,   // dot pixel 1 x 1
	DOT_FILL_RIGHTUP  ,     // dot pixel 2 X 2
} dot_style_t;
#define DOT_STYLE_DFT			( DOT_FILL_AROUND )  //Default dot pilex

/**
 * Line style, solid or dashed
**/
typedef enum {
	LINE_STYLE_SOLID = 0,
	LINE_STYLE_DOTTED,
} line_style_t;

/**
 * Whether the graphic is filled
**/
typedef enum {
	DRAW_FILL_EMPTY = 0,
	DRAW_FILL_FULL,
} draw_fill_t;

/**
 * Custom structure of a time attribute
**/
typedef struct {
	uint8_t		year;  //0000
	uint16_t	month; //1 - 12
	uint16_t	day;   //1 - 30
	uint16_t	hour;  //0 - 23
	uint16_t	min;   //0 - 59
	uint16_t	sec;   //0 - 59
} paint_time_t;
extern paint_time_t __paint_time;

//init and Clear
void paint_new_image(		uint16_t width, 
				uint16_t height, 
				uint16_t rotate, 
				uint16_t color );
void paint_select_image(	uint8_t *image );
void paint_set_rotate(		uint16_t rotate);
void paint_set_mirroring(	uint8_t mirror );
void paint_set_pixel(		uint16_t x_point, 
				uint16_t y_point, 
				uint16_t color );

void paint_clear(		uint16_t color );
void paint_clear_windows(	uint16_t x_start, 
				uint16_t y_start, 
				uint16_t x_end, 
				uint16_t y_end, 
				uint16_t color );

//Drawing
void paint_draw_point(		uint16_t x_point, 
				uint16_t y_point, 
				uint16_t color, 
				dot_pixel_t dot_pixel, 
				dot_style_t dot_fill_way );
void paint_draw_line(		uint16_t x_start, 
				uint16_t y_start, 
				uint16_t x_end, 
				uint16_t y_end, 
				uint16_t color, 
				dot_pixel_t line_width, 
				line_style_t line_style);
void paint_draw_rectangle(	uint16_t x_start, 
				uint16_t y_start, 
				uint16_t x_end, 
				uint16_t y_end, 
				uint16_t color, 
				dot_pixel_t line_width, 
				draw_fill_t filled );
void paint_draw_circle(		uint16_t x_center, 
				uint16_t y_center, 
				uint16_t radius, 
				uint16_t color, 
				dot_pixel_t line_width, 
				draw_fill_t draw_fill );

//Display string
void paint_draw_char(		uint16_t x_point, 
				uint16_t y_point, 
				const char acsii_char, 
				__s_font_t *font, 
				uint16_t color_background, 
				uint16_t color_foreground );
void paint_draw_string(		uint16_t x_start, 
				uint16_t y_start, 
				const char * p_string, 
				__s_font_t *font, 
				uint16_t color_background, 
				uint16_t color_foreground );
void paint_draw_num(		uint16_t x_point, 
				uint16_t y_point, 
				int32_t nummber, 
				__s_font_t *font, 
				uint16_t color_background, 
				uint16_t color_foreground );
void paint_draw_float_num(	uint16_t x_point, 
				uint16_t y_point, 
				double nummber, 
				uint8_t decimal_point, 
				__s_font_t *font, 
				uint16_t color_background, 
				uint16_t color_foreground );
void paint_draw_time(		uint16_t x_start, 
				uint16_t y_start, 
				paint_time_t *time, 
				__s_font_t *font, 
				uint16_t color_background, 
				uint16_t color_foreground );

//pic
void paint_draw_image(		uint8_t *image, 
				uint16_t x_start, 
				uint16_t y_start, 
				uint16_t x_end, 
				uint16_t y_end ); 


#endif  // __GUI_PAINT_H__



