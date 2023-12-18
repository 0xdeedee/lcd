#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#define LCD_WIDTH		( 320 )		//LCD width
#define LCD_HEIGHT		( 240 )		//LCD height

#define LCD_HOST		( SPI2_HOST )

#define PIN_NUM_MISO		( 41 )		// unused, display is write only 
#define PIN_NUM_MOSI		( 21 )		// MOSI
#define PIN_NUM_CLK		( 20 )		// CLOCK
#define PIN_NUM_CS		( 19 )		// Cable Select

#define PIN_NUM_DC		( 45 )
#define PIN_NUM_RST		( 48 )
#define PIN_NUM_BCKL		( 47 )

#define LCD_BK_LIGHT_ON_LEVEL	( 1 )
#define LCD_BK_LIGHT_OFF_LEVEL	( 0 )

#define PARALLEL_LINES		( 16 )

#endif // __DISPLAY_CONFIG_H__

