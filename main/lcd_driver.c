#include <string.h>

#include "lcd_driver.h"


#define LCD_HOST                ( SPI2_HOST )

#define PIN_NUM_MISO            ( 41 )          // unused, display is write only 
#define PIN_NUM_MOSI            ( 21 )          
#define PIN_NUM_CLK             ( 20 )          
#define PIN_NUM_CS              ( 19 )          
                                
#define PIN_NUM_DC              ( 45 )
#define PIN_NUM_RST             ( 48 )
#define PIN_NUM_BCKL            ( 47 )
                                
#define PARALLEL_LINES          ( 16 )

#define LCD_CTRL_BITMASK        ( ( 1ULL << PIN_NUM_DC ) | ( 1ULL << PIN_NUM_RST ) | ( 1ULL << PIN_NUM_BCKL ) )

spi_device_handle_t		__spi;


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


void lcd_cmd( spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active )
{
	esp_err_t			ret;
	spi_transaction_t		t;

	memset( &t, 0, sizeof( t ) );                                   //Zero out the transaction
	t.length = 8;                                                   //Command is 8 bits
	t.tx_buffer = &cmd;                                             //The data is the cmd itself
	t.user = ( void * )0;                                           //D/C needs to be set to 0

	if ( keep_cs_active )
	{
		t.flags = SPI_TRANS_CS_KEEP_ACTIVE;                     //Keep CS active after data transfer
	}
	ret = spi_device_polling_transmit( spi, &t );                   //Transmit!
	assert( ret == ESP_OK );                                        //Should have had no issues.
}

void lcd_data( spi_device_handle_t spi, const uint8_t *data, int len )
{
        esp_err_t                       ret;
        spi_transaction_t               t;

        if ( len == 0 )
        {
                return;
        }

        memset( &t, 0, sizeof( t ) );                                   //Zero out the transaction
        t.length = len * 8;                                             //Len is in bytes, transaction length is in bits.
        t.tx_buffer = data;                                             //Data
        t.user = ( void * )1;                                           //D/C needs to be set to 1
        ret = spi_device_polling_transmit( spi, &t );                   //Transmit!
        assert( ret == ESP_OK );                                        //Should have had no issues.
}


static void lcd_reset()                 
{       
	gpio_set_level( PIN_NUM_RST, 0 );
	vTaskDelay( 100 / portTICK_PERIOD_MS);                          
	gpio_set_level( PIN_NUM_RST, 1 );                               
	vTaskDelay( 100 / portTICK_PERIOD_MS );                         
}

void lcd_init()
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
		lcd_cmd( __spi, lcd_init_cmds[cmd].cmd, false );
		lcd_data( __spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F );
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


void lcd_spi_pre_transfer_callback( spi_transaction_t *t )
{
	int		dc = ( int )t->user;

	gpio_set_level( PIN_NUM_DC, dc );
}

void lcd_init_device()
{
        esp_err_t                               ret;
        spi_bus_config_t                        buscfg = {
                                .miso_io_num = PIN_NUM_MISO,
                                .mosi_io_num = PIN_NUM_MOSI,
                                .sclk_io_num = PIN_NUM_CLK,
                                .quadwp_io_num = -1,
                                .quadhd_io_num = -1,
                                .max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8
                        };
        spi_device_interface_config_t           devcfg = {
                              .clock_speed_hz = 26 * 1000 * 1000,     //Clock out at 26 MHz
//                                .clock_speed_hz = 10 * 1000 * 1000,     //Clock out at 10 MHz
                                .mode = 0,                              //SPI mode 0
                                .spics_io_num = PIN_NUM_CS,             //CS pin
                                .queue_size = 7,                        //We want to be able to queue 7 transactions at a time
                                .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
                        };

        //Initialize the SPI bus
        ret = spi_bus_initialize( LCD_HOST, &buscfg, SPI_DMA_CH_AUTO );
        ESP_ERROR_CHECK( ret );

        //Attach the LCD to the SPI bus
        ret = spi_bus_add_device( LCD_HOST, &devcfg, &__spi);
        ESP_ERROR_CHECK( ret );



}


static void lcd_write_command( spi_device_handle_t spi, uint8_t data )
{
        esp_err_t                       ret;
        static spi_transaction_t        trans;
//        static spi_transaction_t        *rtrans;

	memset( &trans, 0, sizeof( trans ) );
        trans.length = 8;
        trans.user = (void*)0;
        trans.flags = SPI_TRANS_USE_TXDATA;
	trans.tx_data[0]        = data;

	ret = spi_device_polling_transmit( spi, &trans );
//        ret = spi_device_queue_trans( spi, &trans, portMAX_DELAY );
        assert(ret == ESP_OK);

//	ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY);
//        assert(ret == ESP_OK);
}

static void lcd_write_data_byte( spi_device_handle_t spi, uint8_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
//	static spi_transaction_t	*rtrans;

	trans.length		= 8;
	trans.user		= ( void * )1;
	trans.flags		= SPI_TRANS_USE_TXDATA;
	trans.tx_data[0]	= data;

	ret = spi_device_polling_transmit( spi, &trans );
//	ret = spi_device_queue_trans( spi, &trans, portMAX_DELAY );
	assert( ret == ESP_OK );

//	ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY );
//	assert( ret == ESP_OK );
}

static void lcd_write_data_word( spi_device_handle_t spi, uint16_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
//	static spi_transaction_t	*rtrans;

	trans.length		= 16;
	trans.user		= ( void * )1;
	trans.flags		= SPI_TRANS_USE_TXDATA;
//	trans.tx_buffer = &data;
	trans.tx_data[0]	= ( data >> 8 ) & 0xFF;
	trans.tx_data[1]	= data & 0xFF;
//	printf( "--> data %d, %d, %d \n", data, trans.tx_data[0], trans.tx_data[1]);


//	ret = spi_device_polling_transmit( spi, &trans );
	ret = spi_device_queue_trans( spi, &trans, portMAX_DELAY );
	assert( ret == ESP_OK );

//	ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY );
//	assert( ret == ESP_OK );
}


/******************************************************************************
function:	Set the cursor position
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate
	  Xend  :	End UWORD coordinates
	  Yend  :	End UWORD coordinatesen
******************************************************************************/
void lcd_set_window( uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end )
{ 
	lcd_write_command( __spi, 0x2a );
	lcd_write_data_byte( __spi, 0x00 );
	lcd_write_data_byte( __spi, x_start & 0xff );
	lcd_write_data_byte( __spi, ( x_end - 1 ) >> 8 );
	lcd_write_data_byte( __spi, ( x_end - 1 ) & 0xff );

	lcd_write_command( __spi, 0x2b );
	lcd_write_data_byte( __spi, 0x00 );
	lcd_write_data_byte( __spi, y_start & 0xff );
	lcd_write_data_byte( __spi, ( y_end - 1 ) >> 8 );
	lcd_write_data_byte( __spi, ( y_end - 1 ) & 0xff );

	lcd_write_command( __spi, 0x2C );
}

/******************************************************************************
function:	Settings window
parameter	:
	  Xstart: 	Start UWORD x coordinate
	  Ystart:	Start UWORD y coordinate

******************************************************************************/
void lcd_set_cursor( uint16_t x, uint16_t y )
{
	lcd_write_command( __spi, 0x2a );
	lcd_write_data_byte( __spi, x >> 8 );
	lcd_write_data_byte( __spi, x );
	lcd_write_data_byte( __spi, x >> 8 );
	lcd_write_data_byte( __spi, x );

	lcd_write_command( __spi, 0x2b );
	lcd_write_data_byte( __spi, y >> 8 );
	lcd_write_data_byte( __spi, y );
	lcd_write_data_byte( __spi, y >> 8 );
	lcd_write_data_byte( __spi, y );

	lcd_write_command( __spi, 0x2C );
}

/******************************************************************************
function:	Clear screen function, refresh the screen to a certain color
parameter	:
	  Color :		The color you want to clear all the screen
******************************************************************************/
/*
void lcd_clear( uint16_t color )
{
	uint32_t		i;
	uint32_t		j;  	

	lcd_set_window( 0, 0, LCD_WIDTH, LCD_HEIGHT );
	gpio_set_level( PIN_NUM_DC, 1 );
	for( i = 0; i < LCD_WIDTH; i++ )
	{
		for( j = 0; j < LCD_HEIGHT; j++ )
		{
			lcd_write_data_byte( __spi, ( color >> 8 ) & 0xff );
			lcd_write_data_byte( __spi, color );
		}
	 }
}
*/
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
			lcd_write_data_word( __spi, color );
		}
	} 					  	    
}

static void send_line_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}

static void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata)
{
	esp_err_t			ret;
	int				x;
	static spi_transaction_t	trans[6];

//printf( "  [%d] -> {%s} \n", __LINE__, __FUNCTION__ );
    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
	for ( x = 0; x < 6; x++ )
	{
		memset( &trans[x], 0, sizeof( spi_transaction_t ) );
		if ( ( x & 1 ) == 0 ) 
		{
            //Even transfers are commands
			trans[x].length = 8;
			trans[x].user = ( void * )0;
		}
		else 
		{
            //Odd transfers are data
			trans[x].length = 8 * 4;
			trans[x].user= ( void * )1;
		}
		trans[x].flags = SPI_TRANS_USE_TXDATA;
	}

	trans[0].tx_data[0] = 0x2A;           //Column Address Set
	trans[1].tx_data[0] = 0;              //Start Col High
	trans[1].tx_data[1] = 0;              //Start Col Low
	trans[1].tx_data[2] = ( 320 ) >> 8;       //End Col High
	trans[1].tx_data[3] = ( 320 ) & 0xff;     //End Col Low
	trans[2].tx_data[0] = 0x2B;           //Page address set
	trans[3].tx_data[0] = ypos >> 8;        //Start page high
	trans[3].tx_data[1] = ypos & 0xff;      //start page low
	trans[3].tx_data[2] = ( ypos + PARALLEL_LINES ) >> 8;    //end page high
	trans[3].tx_data[3] = ( ypos + PARALLEL_LINES ) & 0xff;  //end page low
	trans[4].tx_data[0] = 0x2C;           //memory write
	trans[5].tx_buffer = linedata;        //finally send the line data
	trans[5].length = 320 * 2 * 8 * PARALLEL_LINES;          //Data length, in bits
	trans[5].flags = 0; //undo SPI_TRANS_USE_TXDATA flag

//printf( "  [%d] -> {%s} \n", __LINE__, __FUNCTION__ );
    //Queue all transactions.
	for (x=0; x<6; x++) 
	{
		ret = spi_device_queue_trans( spi, &trans[x], portMAX_DELAY );
		assert( ret == ESP_OK );
	}

//printf( "  %d \n", __LINE__ );
    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}



void lcd_clear( uint16_t color )
{
	uint16_t		*lines;
	int			frame = 0;
	int			sending_line = -1;
	int			calc_line = 0;

//printf( "  [%d] -> {%s} %d\n", __LINE__, __FUNCTION__, sizeof( lines ) );
//	for ( int i = 0; i < 2; i++ )
//	{
		lines = heap_caps_malloc( 320 * PARALLEL_LINES * sizeof( uint16_t ), MALLOC_CAP_DMA );
//		assert( lines[i] != NULL );
//	}

    //Indexes of the line currently being sent to the LCD and the line we're calculating.

	while( !frame )
	{
		frame++;
		for ( int y = 0; y < 240; y += PARALLEL_LINES )
		{
			memset( lines, color, 320 * PARALLEL_LINES * sizeof( uint16_t ) );
//			pretty_effect_calc_lines( lines[calc_line], y, frame, PARALLEL_LINES );
            //Finish up the sending process of the previous line, if any
//printf( "  %d %d\n", __LINE__, y );
			if ( sending_line != -1 )
				send_line_finish( __spi );
            //Swap sending_line and calc_line
			sending_line ++;
//			calc_line = ( calc_line == 1 ) ? 0: 1;
//printf( "  %d %d\n", __LINE__, y );
            //Send the line we currently calculated.
			send_lines( __spi, y, lines );
//printf( "  %d %d\n", __LINE__, y );

            //The line set is queued up for sending now; the actual sending happens in the
            //background. We can go on to calculate the next line set as long as we do not
            //touch line[sending_line]; the SPI sending process is still reading from that.
		}
	}
	free( lines );
//	free( lines[1] );
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
	lcd_write_data_word( __spi, color );
}




