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

/*
#define LCD_HOST		( SPI2_HOST )

#define PIN_NUM_MISO		( 41 )		// unused, display is write only 
#define PIN_NUM_MOSI		( 21 )
#define PIN_NUM_CLK		( 20 )
#define PIN_NUM_CS		( 19 )

#define PIN_NUM_DC		( 45 )
#define PIN_NUM_RST		( 48 )
#define PIN_NUM_BCKL		( 47 )

#define PARALLEL_LINES		( 16 )
*/

typedef enum {
    LCD_TYPE_ILI = 1,
    LCD_TYPE_ST,
    LCD_TYPE_MAX,
} type_lcd_t;

/*
void lcd_cmd( spi_device_handle_t spi, const uint8_t cmd, bool keep_cs_active )
{
	esp_err_t			ret;
	spi_transaction_t		t;

	memset( &t, 0, sizeof( t ) );					//Zero out the transaction
	t.length = 8;							//Command is 8 bits
	t.tx_buffer = &cmd;						//The data is the cmd itself
	t.user = ( void * )0;						//D/C needs to be set to 0

	if ( keep_cs_active )
	{
		t.flags = SPI_TRANS_CS_KEEP_ACTIVE;			//Keep CS active after data transfer
	}
	ret = spi_device_polling_transmit( spi, &t );			//Transmit!
	assert( ret == ESP_OK );					//Should have had no issues.
}
*/

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
/*
void lcd_data( spi_device_handle_t spi, const uint8_t *data, int len )
trans.tx_data[1]{
	esp_err_t			ret;
	spi_transaction_t		t;

	if ( len == 0 )
	{
		return;
	}

	memset( &t, 0, sizeof( t ) );					//Zero out the transaction
	t.length = len * 8;						//Len is in bytes, transaction length is in bits.
	t.tx_buffer = data;						//Data
	t.user = ( void * )1;						//D/C needs to be set to 1
	ret = spi_device_polling_transmit( spi, &t );			//Transmit!
	assert( ret == ESP_OK );					//Should have had no issues.
}
*/
//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
/*
void lcd_spi_pre_transfer_callback( spi_transaction_t *t )
{
	int				dc = ( int )t->user;

	gpio_set_level( PIN_NUM_DC, dc );
}
*/
/*
static void lcd_reset()
{
	gpio_set_level( PIN_NUM_RST, 0 );
	vTaskDelay( 100 / portTICK_PERIOD_MS);
	gpio_set_level( PIN_NUM_RST, 1 );
	vTaskDelay( 100 / portTICK_PERIOD_MS );
}
*/
#define LCD_CTRL_BITMASK	( ( 1ULL << PIN_NUM_DC ) | ( 1ULL << PIN_NUM_RST ) | ( 1ULL << PIN_NUM_BCKL ) )
/*
//Initialize the display
void lcd_init( spi_device_handle_t spi )
{
	int			cmd = 0;
	const lctrans.tx_data[1]d_init_cmd_t	*lcd_init_cmds;
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
		lcd_cmd(spi, lcd_init_cmds[cmd].cmd, false);
		lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
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
trans.tx_data[1]}
*/
/* To send a set of lines we have to send a command, 2 data bytes, another command, 2 more data bytes and another command
 * before sending the line data itself; a total of 6 transactions. (We can't put all of this in just one transaction
 * because the D/C line needs to be toggled in the middle.)
 * This routine queues these commands up as interrupt transactions so they get
 * sent faster (compared to calling spi_device_transmit several times), and at
 * the mean while the lines for next transactions can get calculated.
 */

/*
static void send_lines( spi_device_handle_t spi, int ypos, uint16_t *data )
{
	esp_err_t			ret;
	int				x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
	static spi_transaction_t	trans[6];

printf( "  %d\n", __LINE__ );
    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
	for ( x = 0; x < 6; x++ )
	{
		memset( &trans[x], 0, sizeof( spi_transaction_t ) );
//		printf( " x %d x & 1 = %d\n", x, x & 1 );
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
			trans[x].user = ( void * )1;
		}
		trans[x].flags = SPI_TRANS_USE_TXDATA;
	}

printf( "  %d\n", __LINE__ );
	trans[0].tx_data[0] = 0x2A;         //Column Address Set
	trans[1].tx_data[0] = 0;            //Start Col High
	trans[1].tx_data[1] = 0;            //Start Col Low
	trans[1].tx_data[2] = ( 320 ) >> 8;   //End Col High
	trans[1].tx_data[3] = ( 320 ) & 0xff; //End Col Low
	trans[2].tx_data[0] = 0x2B;         //Page address set
	trans[3].tx_data[0] = ypos >> 8;    //Start page high
	trans[3].tx_data[1] = ypos & 0xff;  //start page low
	trans[3].tx_data[2] = ( ypos + PARALLEL_LINES ) >> 8; //end page high
	trans[3].tx_data[3] = ( ypos + PARALLEL_LINES ) & 0xff; //end page low
	trans[4].tx_data[0] = 0x2C;         //memory write
	trans[5].tx_buffer = data;      //finally send the line data
	trans[5].length = 320 * 2 * 8 * PARALLEL_LINES;  //Data length, in bits
	trans[5].flags = 0; //undo SPI_TRANS_USE_TXDATA flag

printf( "  %d\n", __LINE__ );
    //Queue all transactions.
	for ( x = 0; x < 6; x++ )
	{
		ret = spi_device_queue_trans( spi, &trans[x], portMAX_DELAY );
		assert( ret == ESP_OK );
	}

printf( "  %d\n", __LINE__ );
    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}

static void send_line_finish( spi_device_handle_t spi )
{
	spi_transaction_t		*rtrans;
	esp_err_t			ret;

	//Wait for all 6 transactions to be done and get back the results.
	for ( int x = 0; x < 6; x++ )
	{
		ret = spi_device_get_trans_result( spi, &rtrans, portMAX_DELAY );
		assert( ret == ESP_OK );
		//We could inspect rtrans now if we received any info back.
		//The LCD is treated as write-only, though.
	}
}

static void LCD_Write_Command( spi_device_handle_t spi, uint8_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
	static spi_transaction_t	*rtrans;

	trans.length = 8;
	trans.user = (void*)0;
	trans.flags = 0;
	trans.tx_data[0] = data;

	ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
	assert(ret == ESP_OK);

	ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
	assert(ret == ESP_OK);
}

static void LCD_WriteData_Byte( spi_device_handle_t spi, uint8_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
	static spi_transaction_t	*rtrans;

	trans.length = 8;
	trans.user = (void*)1;
	trans.flags = 0;
	trans.tx_data[0] = data;

	ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
	assert(ret == ESP_OK);

	ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
	assert(ret == ESP_OK);
}

void LCD_WriteData_Word( spi_device_handle_t spi, uint16_t data )
{
	esp_err_t			ret;
	static spi_transaction_t	trans;
	static spi_transaction_t	*rtrans;

	trans.length = 16;
	trans.user = (void*)1;
	trans.flags = 0;
	trans.tx_data[0] = data >> 8;
	trans.tx_data[0] = data & 0xFF;

	ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
	assert(ret == ESP_OK);

	ret = spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
	assert(ret == ESP_OK);
}



//Simple routine to generate some patterns and send them to the LCD. Don't expect anything too
//impressive. Because the SPI driver handles transactions in the background, we can calculate the next line
//while the previous one is being sent.
static void display_pretty_colors(spi_device_handle_t spi)
{
	uint16_t		*lines[2];
	int			frame = 0;
	int			sending_line = -1;
	int			calc_line = 0;
	uint32_t		lsz = 0;


	lsz = 320 * PARALLEL_LINES * sizeof( uint16_t );
	//Allocate memory for the pixel buffers
	for ( int i = 0; i < 2; i++ )
	{
		lines[i] = heap_caps_malloc( lsz, MALLOC_CAP_DMA );
		assert( lines[i] != NULL );
 	}

	//Indexes of the line currently being sent to the LCD and the line we're calculating.
	while ( 1 )
	{
		frame++;
		for ( int y = 0; y < 240; y += PARALLEL_LINES )
		{
printf( "  %d\n", __LINE__ );
			//Calculate a line.
			pretty_effect_calc_lines( lines[calc_line], y, frame, PARALLEL_LINES );
            		//Finish up the sending process of the previous line, if any
			if ( sending_line != -1 )
			{
				send_line_finish( spi );
			}

printf( "  %d %d\n", __LINE__, sizeof( lines[sending_line] ) );
			//Swap sending_line and calc_line
			sending_line = calc_line;
			calc_line = ( calc_line == 1 ) ? 0 : 1;
			//Send the line we currently calculated.
//			if ( y > 1 && y < 5 )
//			{
				for ( int i = 0; i < lsz/2; i ++)
					lines[sending_line][i] = 0xFFFF;
//				memset( &lines[sending_line], 0xA, sizeof( lines[sending_line] ) );
//			}
printf( "  %d\n", __LINE__ );
			send_lines( spi, y, lines[sending_line] );
printf( "  %d\n", __LINE__ );
			//The line set is queued up for sending now;
			// the actual sending happens in the
			// background. We can go on to calculate the next 
			// line set as long as we do not
			// touch line[sending_line]; the SPI sending 
			// process is still reading from that.
		}
	}
}
*/
void app_main(void)
{
/*
	esp_err_t				ret;
	spi_device_handle_t			spi;
	spi_bus_config_t			buscfg = {
				.miso_io_num = PIN_NUM_MISO,
				.mosi_io_num = PIN_NUM_MOSI,
				.sclk_io_num = PIN_NUM_CLK,
				.quadwp_io_num = -1,
				.quadhd_io_num = -1,
				.max_transfer_sz = PARALLEL_LINES * 320 * 2 + 8
			};
	spi_device_interface_config_t		devcfg = {
//				.clock_speed_hz = 26 * 1000 * 1000,     //Clock out at 26 MHz
				.clock_speed_hz = 10 * 1000 * 1000,     //Clock out at 10 MHz
				.mode = 0,                              //SPI mode 0
				.spics_io_num = PIN_NUM_CS,             //CS pin
				.queue_size = 7,                        //We want to be able to queue 7 transactions at a time
				.pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
			};

	//Initialize the SPI bus
	ret = spi_bus_initialize( LCD_HOST, &buscfg, SPI_DMA_CH_AUTO );
	ESP_ERROR_CHECK( ret );
*/
	//Attach the LCD to the SPI bus
//	ret = spi_bus_add_device( LCD_HOST, &devcfg, &spi);
//	ESP_ERROR_CHECK( ret );
	lcd_init_device();
	//Initialize the LCD
	lcd_init();

	//Initialize the effect displayed
//	ret = pretty_effect_init();
//	ESP_ERROR_CHECK( ret );

paint_new_image(LCD_WIDTH, LCD_HEIGHT, 0, ~WHITE);
paint_clear(~WHITE);
paint_clear(~BLACK);
paint_clear(~WHITE);
//paint_draw_rectangle( 0, 0, 320, 240, WHITE, DOT_PIXEL_8X8, DRAW_FILL_FULL );
//paint_clear_windows( 0, 0, 320, 240, BLACK );
printf( "  %d\n", __LINE__ );
//paint_draw_string(30, 10, "123", &__font_24, YELLOW, RED);
paint_draw_string(30, 3, "123", &__font_24, ~WHITE, ~BLACK);
printf( "  %d\n", __LINE__ );
paint_draw_string(30, 34, "ABC", &__font_24, ~BLUE, ~CYAN);
printf( "  %d\n", __LINE__ );

paint_draw_rectangle(125, 10, 225, 58, ~RED,  DOT_PIXEL_2X2,DRAW_FILL_EMPTY);
paint_draw_line(125, 10, 225, 58, ~MAGENTA,   DOT_PIXEL_2X2,LINE_STYLE_SOLID);
paint_draw_line(225, 10, 125, 58, ~MAGENTA,   DOT_PIXEL_2X2,LINE_STYLE_SOLID);

paint_draw_circle(150,100, 25, ~BLUE,   DOT_PIXEL_2X2,   DRAW_FILL_EMPTY);
paint_draw_circle(180,100, 25, ~BLACK,  DOT_PIXEL_2X2,   DRAW_FILL_EMPTY);
paint_draw_circle(210,100, 25, ~RED,    DOT_PIXEL_2X2,   DRAW_FILL_EMPTY);
paint_draw_circle(165,125, 25, ~YELLOW, DOT_PIXEL_2X2,   DRAW_FILL_EMPTY);
paint_draw_circle(195,125, 25, ~GREEN,  DOT_PIXEL_2X2,   DRAW_FILL_EMPTY);


paint_draw_line(0, 2, 320, 2, ~MAGENTA,   DOT_PIXEL_2X2,LINE_STYLE_SOLID);
paint_draw_line(0, 29, 320, 29, ~MAGENTA,   DOT_PIXEL_2X2,LINE_STYLE_SOLID);

paint_draw_string(30, 3, "234", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "334", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "434", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "534", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "634", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "734", &__font_24, ~WHITE, ~BLACK);
paint_draw_string(30, 3, "834", &__font_24, ~WHITE, ~BLACK);

printf( "  %d\n", __LINE__ );
	//Go do nice stuff.
//	display_pretty_colors( __spi );
}




