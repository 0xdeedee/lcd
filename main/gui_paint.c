#define RANDOM_MAX		( 0x7FFFFFFF )
#define DTOSTR_ALWAYS_SIGN	( 0x01 )       /* put '+' or ' ' for positives */
#define DTOSTR_PLUS_SIGN	( 0x02 )        /* put '+' rather than ' ' */
#define DTOSTR_UPPERCASE	( 0x04 )        /* put 'E' rather 'e' */
#define EXIT_SUCCESS		( 0 )
#define EXIT_FAILURE		( 1 )



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>
#include "gui_paint.h"

#define ARRAY_LEN		( 50 )

volatile paint_t	__paint;

/******************************************************************************
  function: Create Image
  parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    color   :   Whether the picture is inverted
******************************************************************************/
void paint_new_image(		uint16_t width,
				uint16_t height,
				uint16_t rotate,
				uint16_t color )
{
	__paint.width_memory = width;
	__paint.height_memory = height;
	__paint.color = color;
	__paint.width_byte = width;
	__paint.height_byte = height;
	__paint.rotate = rotate;
	__paint.mirror = MIRROR_NONE;

	if ( ( rotate == ROTATE_0 ) || ( rotate == ROTATE_180) ) 
	{
		__paint.width = width;
		__paint.height = height;
	} 
	else 
	{
		__paint.width = height;
		__paint.height = width;
	}
}

/******************************************************************************
  function: Select Image Rotate
  parameter:
    Rotate   :   0,90,180,270
******************************************************************************/
void paint_set_rotate( uint16_t rotate )
{
	if (		( rotate == ROTATE_0 ) 
		|| 	( rotate == ROTATE_90 ) 
		||	( rotate == ROTATE_180 ) 
		||	( rotate == ROTATE_270) ) 
	{
		//Debug("Set image Rotate %d\r\n", Rotate);
		__paint.rotate = rotate;
	}
	else 
	{
		//Debug("rotate = 0, 90, 180, 270\r\n");
		//  exit(0);
	}
}

/******************************************************************************
  function: Select Image mirror
  parameter:
    mirror   :       Not mirror,Horizontal mirror,Vertical mirror,Origin mirror
******************************************************************************/
void paint_set_mirroring( uint8_t mirror )
{
	if (		( mirror == MIRROR_NONE ) 
		||	( mirror == MIRROR_HORIZONTAL ) 
		||	( mirror == MIRROR_VERTICAL )
		||	( mirror == MIRROR_ORIGIN )  ) 
	{
		//Debug("mirror image x:%s, y:%s\r\n", (mirror & 0x01) ? "mirror" : "none", ((mirror >> 1) & 0x01) ? "mirror" : "none");
		 __paint.mirror = mirror;
	} else {
		//Debug("mirror should be MIRROR_NONE, MIRROR_HORIZONTAL, MIRROR_VERTICAL or MIRROR_ORIGIN\r\n");
		//exit(0);
	}
}

/******************************************************************************
  function: Draw Pixels
  parameter:
    x_point  :   At point X
    y_point  :   At point Y
    color   :   __paint.d colors
******************************************************************************/
void paint_set_pixel(		uint16_t x_point,
				uint16_t y_point,
				uint16_t color )
{
	uint16_t		x;
	uint16_t		y;

	if ( ( x_point > __paint.width ) || ( y_point > __paint.height ) ) 
	{
		//Debug("Exceeding display boundaries\r\n");
		return;
	}

	switch (__paint.rotate) {
		case 0:
				x = x_point;
				y = y_point;
			break;
		case 90:
				x = __paint.width_memory - y_point - 1;
				y = x_point;
			break;
		case 180:
				x = __paint.width_memory - x_point - 1;
				y = __paint.height_memory - y_point - 1;
			break;
		case 270:
				x = y_point;
				y = __paint.height_memory - x_point - 1;
			break;
		default:
			return;
	}

	switch (__paint.mirror) {
		case MIRROR_NONE:
			break;
		case MIRROR_HORIZONTAL:
				x = __paint.width_memory - x - 1;
			break;
		case MIRROR_VERTICAL:
				y = __paint.height_memory - y - 1;
			break;
		case MIRROR_ORIGIN:
				x = __paint.width_memory - x - 1;
				y = __paint.height_memory - y - 1;
			break;
		default:
			return;
	}

	//printf("x = %d, y = %d\r\n", X, Y);
	if ( ( x > __paint.width_memory) || ( y > __paint.height_memory) )
	{
		//Debug("Exceeding display boundaries\r\n");
		return;
	}

	// UDOUBLE Addr = X / 8 + Y * __paint.widthByte;
	lcd_draw_paint( x, y, color );
}

/******************************************************************************
  function: Clear the color of the picture
  parameter:
    color   :   __paint.d colors
******************************************************************************/
void paint_clear( uint16_t color )
{
	lcd_set_window( 0, 0, __paint.width_byte , __paint.height_byte );

	for ( uint16_t y = 0; y < __paint.height_byte; y++ ) 
	{
		for ( uint16_t x = 0; x < __paint.width_byte; x++ ) 
		{//8 pixel =  1 byte
			lcd_write_data_word( color );
		}
	}
}

/******************************************************************************
  function: Clear the color of a window
  parameter:
    x_start :   x starting point
    y_start :   Y starting point
    x_end   :   x end point
    y_end   :   y end point
******************************************************************************/
void paint_clear_windows(	uint16_t x_start,
				uint16_t y_start,
				uint16_t x_end,
				uint16_t y_end,
				uint16_t color )
{
	uint16_t		x;
	uint16_t		y;

	for ( y = y_start; y < y_end; y++ ) 
	{
		for ( x = x_start; x < x_end; x++ ) 
		{//8 pixel =  1 byte
			paint_set_pixel( x, y, color );
		}
	}
}

/******************************************************************************
function:	Draw Point(x_point, y_point) Fill the color
parameter:
    x_point		:   The x_point coordinate of the point
    y_point		:   The y_point coordinate of the point
    color		:   Set color
    Dot_Pixel	:	point size
******************************************************************************/
void paint_draw_point(		uint16_t x_point,
				uint16_t y_point,
				uint16_t color,
				dot_pixel_t dot_pixel,
				dot_style_t dot_fill_way )
{
	int16_t			x_dir_num;
	int16_t			y_dir_num;

	if ( ( x_point > __paint.width ) || ( y_point > __paint.height) )
	{
		//Debug("paint_draw_point Input exceeds the normal display range\r\n");
		return;
	}

	if ( dot_fill_way == DOT_FILL_AROUND ) 
	{
		for ( x_dir_num = 0; x_dir_num < 2 * dot_pixel - 1; x_dir_num++ ) 
		{
			for ( y_dir_num = 0; y_dir_num < 2 * dot_pixel - 1; y_dir_num++) 
			{
				if (		( ( int )( x_point + x_dir_num - dot_pixel ) < 0 ) 
					||	( ( int )( y_point + y_dir_num - dot_pixel ) < 0 ) ) 
					break;
				paint_set_pixel( x_point + x_dir_num - dot_pixel, y_point + y_dir_num - dot_pixel, color );
			}
		}
	} 
	else 
	{
		for ( x_dir_num = 0; x_dir_num <  dot_pixel; x_dir_num++ ) 
		{
			for ( y_dir_num = 0; y_dir_num <  dot_pixel; y_dir_num++ ) 
			{
				paint_set_pixel( x_point + x_dir_num - 1, y_point + y_dir_num - 1, color );
			}
		}
	}
}

/******************************************************************************
function:	Draw a line of arbitrary slope
parameter:
    x_start ：Starting x_point point coordinates
    y_start ：Starting x_point point coordinates
    x_end   ：End point x_point coordinate
    y_end   ：End point y_point coordinate
    color  ：The color of the line segment
******************************************************************************/

void paint_draw_line(           uint16_t x_start,
                                uint16_t y_start,
                                uint16_t x_end,
                                uint16_t y_end,
                                uint16_t color,
                                dot_pixel_t line_width,
                                line_style_t line_style)
{
	if (		( x_start > __paint.width ) 
		||	( y_start > __paint.height ) 
		||	( x_end > __paint.width ) 
		||	( y_end > __paint.height ) ) 
	{
		//Debug("__paint.DrawLine Input exceeds the normal display range\r\n");
		return;
	}

	uint16_t		x_point = x_start;
	uint16_t		y_point = y_start;
	int			dx = (int)x_end - (int)x_start >= 0 ? x_end - x_start : x_start - x_end;
	int			dy = (int)y_end - (int)y_start <= 0 ? y_end - y_start : y_start - y_end;
	int			x_add_way = x_start < x_end ? 1 : -1;
	int			y_add_way = y_start < y_end ? 1 : -1;
	int			esp = dx + dy;
	char			dotted_len = 0;

	for (;;)
	{
		dotted_len++;
		if ( ( line_style == LINE_STYLE_DOTTED ) && ( dotted_len % 3 == 0 ) )
		{
			paint_draw_point( x_point, y_point, IMAGE_BACKGROUND, line_width, DOT_STYLE_DFT );
			dotted_len = 0;
		}
		else 
		{
			paint_draw_point( x_point, y_point, color, line_width, DOT_STYLE_DFT );
		}

		if ( 2 * esp >= dy ) 
		{
			if ( x_point == x_end )

				break;
			esp += dy;
			x_point += x_add_way;
		}

		if ( 2 * esp <= dx ) 
		{
			if (y_point == y_end)

				break;
			esp += dx;
			y_point += y_add_way;
		}
	}
}

/******************************************************************************
function:	Draw a rectangle
parameter:
    x_start ：Rectangular  Starting x_point point coordinates
    y_start ：Rectangular  Starting x_point point coordinates
    x_end   ：Rectangular  End point x_point coordinate
    y_end   ：Rectangular  End point y_point coordinate
    color  ：The color of the Rectangular segment
    Filled : Whether it is filled--- 1 solid 0：empty
******************************************************************************/


void paint_draw_rectangle(	uint16_t x_start,
				uint16_t y_start,
				uint16_t x_end,
				uint16_t y_end,
				uint16_t color,
				dot_pixel_t line_width,
				draw_fill_t filled )
{
	if (		( x_start > __paint.width ) 
		||	( y_start > __paint.height ) 
		||	( x_end > __paint.width ) 
		||	( y_end > __paint.height ) )
	{
		//Debug("Input exceeds the normal display range\r\n");
		return;
	}

	if ( filled ) 
	{
		uint16_t		y_point;

		for( y_point = y_start; y_point < y_end; y_point++ ) 
		{
			paint_draw_line( x_start, y_point, x_end, y_point, color ,line_width, LINE_STYLE_SOLID );
		}
	} 
	else 
	{
		paint_draw_line( x_start, y_start, x_end, y_start, color ,line_width, LINE_STYLE_SOLID );
		paint_draw_line( x_start, y_start, x_start, y_end, color ,line_width, LINE_STYLE_SOLID );
		paint_draw_line( x_end, y_end, x_end, y_start, color ,line_width, LINE_STYLE_SOLID );
		paint_draw_line( x_end, y_end, x_start, y_end, color ,line_width, LINE_STYLE_SOLID );
	}
}

/******************************************************************************
function:	Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    x_center  ：Center X coordinate
    y_center  ：Center Y coordinate
    Radius    ：circle Radius
    color     ：The color of the ：circle segment
    Filled    : Whether it is filled: 1 filling 0：Do not
******************************************************************************/

void paint_draw_circle(		uint16_t x_center,
				uint16_t y_center,
				uint16_t radius,
				uint16_t color,
				dot_pixel_t line_width,
				draw_fill_t draw_fill )
{
	int16_t			x_current = 0;
	int16_t			y_current = radius;
	int16_t			esp = 3 - ( radius << 1 );
	int16_t			s_count_y;

	if ( ( x_center > __paint.width ) || ( y_center >= __paint.height ) ) 
	{
		//Debug("__paint.DrawCircle Input exceeds the normal display range\r\n");
		return;
	}

	if ( draw_fill == DRAW_FILL_FULL) 
	{
		while ( x_current <= y_current ) 
		{ //Realistic circles
			for ( s_count_y = x_current; s_count_y <= y_current; s_count_y ++ ) 
			{
				paint_draw_point(x_center + x_current, y_center + s_count_y, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//1
				paint_draw_point(x_center - x_current, y_center + s_count_y, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//2
				paint_draw_point(x_center - s_count_y, y_center + x_current, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//3
				paint_draw_point(x_center - s_count_y, y_center - x_current, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//4
				paint_draw_point(x_center - x_current, y_center - s_count_y, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//5
				paint_draw_point(x_center + x_current, y_center - s_count_y, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//6
				paint_draw_point(x_center + s_count_y, y_center - x_current, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//7
				paint_draw_point(x_center + s_count_y, y_center + x_current, color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
			}
			if (esp < 0 )
				esp += 4 * x_current + 6;
			else 
			{
				esp += 10 + 4 * ( x_current - y_current );
				y_current --;
			}
			x_current ++;
		}
	} 
	else 
	{ //Draw a hollow circle
		while (x_current <= y_current )
		{
			paint_draw_point(x_center + x_current, y_center + y_current, color, line_width, DOT_STYLE_DFT);//1
			paint_draw_point(x_center - x_current, y_center + y_current, color, line_width, DOT_STYLE_DFT);//2
			paint_draw_point(x_center - y_current, y_center + x_current, color, line_width, DOT_STYLE_DFT);//3
			paint_draw_point(x_center - y_current, y_center - x_current, color, line_width, DOT_STYLE_DFT);//4
			paint_draw_point(x_center - x_current, y_center - y_current, color, line_width, DOT_STYLE_DFT);//5
			paint_draw_point(x_center + x_current, y_center - y_current, color, line_width, DOT_STYLE_DFT);//6
			paint_draw_point(x_center + y_current, y_center - x_current, color, line_width, DOT_STYLE_DFT);//7
			paint_draw_point(x_center + y_current, y_center + x_current, color, line_width, DOT_STYLE_DFT);//0

			if ( esp < 0 )
				esp += 4 * x_current + 6;
			else 
			{
				esp += 10 + 4 * ( x_current - y_current );
				y_current --;
			}
			x_current ++;
		}
	}
}

/******************************************************************************
  function: Show English characters
  parameter:
    x_point           ：X coordinate
    y_point           ：Y coordinate
    Acsii_Char       ：To display the English characters
    Font             ：A structure pointer that displays a character size
    color_background : Select the background color of the English character
    color_foreground : Select the foreground color of the English character
******************************************************************************/

void paint_draw_char(		uint16_t x_point,
				uint16_t y_point,
				const char acsii_char,
				__s_font_t *font,
				uint16_t color_background,
				uint16_t color_foreground )
{
	uint16_t		page;
	uint16_t		column;

	if ( ( x_point > __paint.width ) || ( y_point > __paint.height ) )
	{
		//Debug("paint_draw_char Input exceeds the normal display range\r\n");
		return;
	}

	uint32_t char_offset = ( acsii_char - ' ' ) * font->height * ( font->width / 8 + ( font->width % 8 ? 1 : 0 ) );
	const unsigned char *ptr = &font->table[char_offset];

	for ( page = 0; page < font->height; page ++ )
	{
		for ( column = 0; column < font->width; column ++ ) 
		{

			//To determine whether the font background color and screen background color is consistent
			if (FONT_BACKGROUND == color_background)
			{
				if ( ( *ptr ) & ( 0x80 >> ( column % 8 ) ) )
					paint_set_pixel ( x_point + column, y_point + page, color_foreground );
			}
			else
			{
				if ( ( *ptr ) & ( 0x80 >> ( column % 8 ) ) ) 
				{
					paint_set_pixel ( x_point + column, y_point + page, color_foreground );
	        		} 
				else 
				{
					paint_set_pixel ( x_point + column, y_point + page, color_background );
				}
			}
			//One pixel is 8 bits
			if ( column % 8 == 7 ) 
			{
				ptr++;
			}
    		}/* Write a line */
		if ( font->width % 8 != 0 ) 
		{
			ptr++;
		}
	}/* Write all */
}

/******************************************************************************
  function: Display the string
  parameter:
    x_start           ：X coordinate
    y_start           ：Y coordinate
    pString          ：The first address of the English string to be displayed
    Font             ：A structure pointer that displays a character size
    color_background : Select the background color of the English character
    color_foreground : Select the foreground color of the English character
******************************************************************************/
void paint_draw_string(         uint16_t x_start,
                                uint16_t y_start,
                                const char *p_string,
                                __s_font_t *font,
                                uint16_t color_background,
                                uint16_t color_foreground )
{
	uint16_t		x_point = x_start;
	uint16_t		y_point = y_start;

	if ( ( x_start > __paint.width ) || ( y_start > __paint.height ) ) 
	{
		//Debug("paint_draw_string Input exceeds the normal display range\r\n");
		return;
	}

	while ( *p_string != '\0') 
	{
		//if X direction filled , reposition to(x_start,y_point),y_point is Y direction plus the Height of the character
		if ( ( x_point + font->width ) > __paint.width )
		{
			x_point = x_start;
			y_point += font->height;
		}

		// If the Y direction is full, reposition to(x_start, y_start)
		if ( ( y_point  + font->height ) > __paint.height ) 
		{
			x_point = x_start;
			y_point = y_start;
		}
		paint_draw_char( x_point, y_point, * p_string, font, color_background, color_foreground);

		//The next character of the address
		p_string ++;

		//The next word of the abscissa increases the font of the broadband
		x_point += font->width;
	}
}


/******************************************************************************
  function: Display nummber
  parameter:
    x_start           ：X coordinate
    y_start           : Y coordinate
    Nummber          : The number displayed
    Font             ：A structure pointer that displays a character size
    color_background : Select the background color of the English character
    color_foreground : Select the foreground color of the English character
******************************************************************************/
void paint_draw_num(		uint16_t x_point,
				uint16_t y_point,
				int32_t nummber,
				__s_font_t *font,
				uint16_t color_background,
				uint16_t color_foreground )
{
	int16_t			num_bit = 0;
	int16_t			str_bit = 0;
	uint8_t			str_array[ARRAY_LEN] = {0};
	uint8_t			num_array[ARRAY_LEN] = {0};
	uint8_t			*pstr = str_array;

	if ( x_point > __paint.width || y_point > __paint.height ) 
	{
		return;
	}

	//Converts a number to a string
	do
	{
		num_array[num_bit] = nummber % 10 + '0';
		num_bit++;
		nummber /= 10;
	} while ( nummber );

	//The string is inverted
	while ( num_bit > 0 ) 
	{
		str_array[str_bit] = num_array[num_bit - 1];
		str_bit ++;
		num_bit --;
	}

	//show
	paint_draw_string( x_point, y_point, ( const char * ) pstr, font, color_background, color_foreground );
}

/******************************************************************************
function:	Display float number
parameter:
    x_start           ：X coordinate
    y_start           : Y coordinate
    Nummber          : The float data that you want to display
	Decimal_Point	 : Show decimal places
    Font             ：A structure pointer that displays a character size
    color            : Select the background color of the English character
******************************************************************************/
void paint_draw_float_num(	uint16_t x_point,
				uint16_t y_point,
				double nummber,
				uint8_t decimal_point,
				__s_font_t *font,
				uint16_t color_background,
				uint16_t color_foreground )
{
	char			str[ARRAY_LEN] = {0};
	char			*pstr = NULL;
#warning "Fic MEEEE !!!!!!!!!11"
//	dtostrf( nummber, 0, decimal_point + 2, str );
	pstr = ( char * )malloc( ( strlen( str ) ) * sizeof( char ) );
	if ( NULL == pstr )
		return;

	memcpy( pstr, str,( strlen( str ) - 2 ) );
	*( pstr + strlen( str ) -1 ) = '\0';
	if( ( *( pstr + strlen( str ) - 3 ) ) =='.' )
	{
		*( pstr + strlen( str ) - 3 ) = '\0';
	}

	//show
	paint_draw_string( x_point, y_point, ( const char * )pstr, font, color_foreground, color_background );
	free( pstr );
	pstr = NULL;
}

/******************************************************************************
  function: Display time
  parameter:
    x_start           : X coordinate
    y_start           : Y coordinate
    pTime            : Time-related structures
    Font             : A structure pointer that displays a character size
    color            : Select the background color of the English character
******************************************************************************/
void paint_draw_time(		uint16_t x_start,
				uint16_t y_start,
				paint_time_t *time,
				__s_font_t *font,
				uint16_t color_background,
				uint16_t color_foreground )
{
	uint8_t			value[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	uint16_t		dx = font->width;

	//Write data into the cache
	paint_draw_char( x_start                           , y_start, value[time->hour / 10], font, color_background, color_foreground);
	paint_draw_char( x_start + dx                      , y_start, value[time->hour % 10], font, color_background, color_foreground);
	paint_draw_char( x_start + dx  + dx / 4 + dx / 2   , y_start, ':'                   , font, color_background, color_foreground);
	paint_draw_char( x_start + dx * 2 + dx / 2         , y_start, value[time->min / 10] , font, color_background, color_foreground);
	paint_draw_char( x_start + dx * 3 + dx / 2         , y_start, value[time->min % 10] , font, color_background, color_foreground);
	paint_draw_char( x_start + dx * 4 + dx / 2 - dx / 4, y_start, ':'                   , font, color_background, color_foreground);
	paint_draw_char( x_start + dx * 5                  , y_start, value[time->sec / 10] , font, color_background, color_foreground);
	paint_draw_char( x_start + dx * 6                  , y_start, value[time->sec % 10] , font, color_background, color_foreground);
}

/******************************************************************************
  function: Display image
  parameter:
    image            ：Image start address
    xStart           : X starting coordinates
    yStart           : Y starting coordinates
    xEnd             ：Image width
    yEnd             : Image height
******************************************************************************/
paint_draw_image(		const uint8_t *image,
				uint16_t x_start,
				uint16_t y_start,
				uint16_t x_end,
				uint16_t y_end )
{
	int		i;
	int		j;
	uint16_t	color;

	for ( j = 0; j < x_end; j++ )
	{
		for ( i = 0; i < y_end; i++ )
		{
			if ( ( x_start + i < LCD_WIDTH ) && ( y_start + j < LCD_HEIGHT ) ) //Exceeded part does not display
			{
				color = ( image + j * y_end * 2 + i * 2 + 1 ) << 8 | ( image + j * y_end * 2 + i * 2  );
				paint_set_pixel( x_start + i, y_start + j, color );
			}
		}
	}

}
