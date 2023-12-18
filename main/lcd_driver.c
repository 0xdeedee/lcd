#include "lcd_driver.h"


#define LCD_CTRL_BITMASK        ( ( 1ULL << PIN_NUM_DC ) | ( 1ULL << PIN_NUM_RST ) | ( 1ULL << PIN_NUM_BCKL ) )

DRAM_ATTR static const lcd_init_cmd_t			st_init_cmds[] = {
	// Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0
	{ 0x36, { ( 1 << 5 ) | ( 1 << 6 ) }, 1 },
	// Interface Pixel Format, 16bits/pixel for RGB/MCU interface
	{ 0x3A, { 0x55 }, 1 },
	// Porch Setting
	{ 0xB2, { 0x0c, 0x0c, 0x00, 0x33, 0x33 }, 5 },
	// Gate Control, Vgh=13.65V, Vgl=-10.43V
	{ 0xB7, { 0x45 }, 1 },
	// VCOM Setting, VCOM=1.175V
	{ 0xBB, { 0x2B }, 1 },
	// LCM Control, XOR: BGR, MX, MH
	{ 0xC0, { 0x2C }, 1 },
	// VDV and VRH Command Enable, enable=1
	{ 0xC2, { 0x01, 0xff }, 2 },
	// VRH Set, Vap=4.4+...
	{ 0xC3, { 0x11 }, 1 },
	// VDV Set, VDV=0
	{ 0xC4, { 0x20 }, 1 },
	// Frame Rate Control, 60Hz, inversion=0
	{ 0xC6, { 0x0f }, 1 },
	// Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V
	{ 0xD0, { 0xA4, 0xA1 }, 1 },
	// Positive Voltage Gamma Control
	{ 0xE0, { 0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19 }, 14 },
	// Negative Voltage Gamma Control
	{ 0xE1, { 0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19 }, 14 },
	// Sleep Out
	{ 0x11, { 0 }, 0x80 },
	// Display On
	{ 0x29, { 0 }, 0x80 },
	{ 0, { 0 }, 0xff }
};



static void lcd_reset()                 
{       
	gpio_set_level( PIN_NUM_RST, 0 );
	vTaskDelay( 100 / portTICK_PERIOD_MS);                          
	gpio_set_level( PIN_NUM_RST, 1 );                               
	vTaskDelay( 100 / portTICK_PERIOD_MS );                         
}

void lcd_init( spi_device_handle_t spi )
{
	int			cmd = 0;
	const lcd_init_cmd_t	*lcd_init_cmds;
	gpio_config_t		io_conf = {};

	//Initialize non-SPI GPIOs
	io_conf.pin_bit_mask = LCD_CTRL_BITMASK;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pull_up_en = true;

	gpio_config( &io_conf );

	//Reset the display
	lcd_reset();

	printf( "LCD ST7789V initialization.\n" );
	lcd_init_cmds = st_init_cmds;

	//Send all the commands
	while ( lcd_init_cmds[cmd].databytes != 0xff )
	{
		lcd_cmd( spi, lcd_init_cmds[cmd].cmd, false );
		lcd_data( spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F );
		if ( lcd_init_cmds[cmd].databytes & 0x80 )
		{
			vTaskDelay( 100 / portTICK_PERIOD_MS );
		}
		cmd++;
	}

	//Enable backlight
	gpio_set_level( PIN_NUM_BCKL, 0 );
	vTaskDelay( 1000 / portTICK_PERIOD_MS );

	gpio_set_level( PIN_NUM_BCKL, 1 );
	vTaskDelay( 1000 / portTICK_PERIOD_MS );

	printf( "LCD ST7789V initialization finished!!!\n" );
}


static void lcd_write_command( spi_device_handle_t spi, uint8_t data )
{
        esp_err_t                       ret;
        static spi_transaction_t        trans;
        static spi_transaction_t        *rtrans;

        trans.length = 8;
        trans.user = (void*)0;
        trans.flags = 0;
        trans.tx_data[0] = data;

        ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
        assert(ret == ESP_OK);

        ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret == ESP_OK);
}

static void lcd_write_data_byte( spi_device_handle_t spi, uint8_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
	static spi_transaction_t	*rtrans;

	trans.length		= 8;
	trans.user		= ( void * )1;
	trans.flags		= 0;
	trans.tx_data[0]	= data;

	ret = spi_device_queue_trans( spi, &trans, portMAX_DELAY );
	assert( ret == ESP_OK );

	ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY );
	assert( ret == ESP_OK );
}

void lcd_write_data_word( spi_device_handle_t spi, uint16_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
	static spi_transaction_t	*rtrans;

	trans.length		= 16;
	trans.user		= ( void * )1;
	trans.flags		= 0;
	trans.tx_data[0]	= data >> 8;
	trans.tx_data[0]	= data & 0xFF;

	ret = spi_device_queue_trans( spi, &trans, portMAX_DELAY );
	assert( ret == ESP_OK );

	ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY );
	assert( ret == ESP_OK );
}


/******************************************************************************
function:	Set the cursor position
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate
	  Xend  :	End UWORD coordinates
	  Yend  :	End UWORD coordinatesen
******************************************************************************/
void lcd_set_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end );
{ 
	lcd_write_command( 0x2a );
	lcd_write_data_byte( 0x00 );
	lcd_write_data_byte( x_start & 0xff );
	lcd_write_data_byte( ( x_end - 1 ) >> 8 );
	lcd_write_data_byte( ( x_end - 1 ) & 0xff );

	lcd_write_command( 0x2b );
	lcd_write_data_byte( 0x00 );
	lcd_write_data_byte( y_start & 0xff );
	lcd_write_data_byte( ( y_end - 1 ) >> 8 );
	lcd_write_data_byte( ( y_end - 1 ) & 0xff );

	lcd_write_command( 0x2C );
}

/******************************************************************************
function:	Settings window
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate

******************************************************************************/
void lcd_set_cursor( uint16_t x, uint16_t y )
{ 
	lcd_write_command( 0x2a );
	lcd_write_data_byte( x >> 8 );
	lcd_write_data_byte( x );
	lcd_write_data_byte( x >> 8 );
	lcd_write_data_byte( x );

	lcd_write_command( 0x2b );
	lcd_write_data_byte( y >> 8 );
	lcd_write_data_byte( y );
	lcd_write_data_byte( y >> 8 );
	lcd_write_data_byte( y );

	lcd_write_command( 0x2C );
}

/******************************************************************************
function:	Clear screen function, refresh the screen to a certain color
parameter	:
	  Color :		The color you want to clear all the screen
******************************************************************************/
void lcd_clear( uint16_t color )
{
	uint32_t		i;
	uint32_t		j;  	

	LCD_SetWindow( 0, 0, LCD_WIDTH, LCD_HEIGHT );
	DEV_Digital_Write( DEV_DC_PIN, 1 );
	for( i = 0; i < LCD_WIDTH; i++ )
	{
		for( j = 0; j < LCD_HEIGHT; j++ )
		{
			lcd_write_data_byte( ( color >> 8 ) & 0xff );
			lcd_write_data_byte( color );
		}
	 }
}

/******************************************************************************
function:	Refresh a certain area to the same color
parameter	:
	  Xstart: Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate
	  Xend  :	End UWORD coordinates
	  Yend  :	End UWORD coordinates
	  color :	Set the color
******************************************************************************/
void lcd_clear_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color )
{          
	uint16_t		i;
	uint16_t		j; 

	lcd_set_window( x_start, y_start, x_end - 1, y_end - 1 );
	for( i = y_start; i <= y_end - 1; i++ )
	{													   	 	
		for( j = x_start; j <= x_end - 1; j++ )
		{
			lcd_write_data_word( color );
		}
	} 					  	    
}

/******************************************************************************
function: Draw a point
parameter	:
	    X	: 	Set the X coordinate
	    Y	:	Set the Y coordinate
	  Color :	Set the color
******************************************************************************/
void lcd_draw_paint( uint16_t x, uint16_t y, uint16_t color )
{
	lcd_set_cursor( x, y );
	lcd_write_data_word( color );
}




