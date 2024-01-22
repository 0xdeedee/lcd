#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdio.h>
#include <stdint.h>


#define MAX_HEIGHT_FONT		( 24 )
#define MAX_WIDTH_FONT		( 17 )
#define OFFSET_BITMAP		( 54 )


#define __FONT_8_ENABLED__	( 0 )
#define __FONT_12_ENABLED__	( 0 )
#define __FONT_16_ENABLED__	( 0 )
#define __FONT_20_ENABLED__	( 0 )
#define __FONT_24_ENABLED__	( 1 )



typedef struct __s_font {
	const uint8_t		*table;
	uint16_t		width;
	uint16_t		height;
} __s_font_t;

/*
#if (1 == __FONT_8_ENABLED__ )
	#include "font8.h"
#endif // __FONT_8_ENABLED__

#if (1 == __FONT_12_ENABLED__ )
	#include "font12.h"
#endif // __FONT_12_ENABLED__

#if (1 == __FONT_16_ENABLED__ )
	#include "font16.h"
#endif // __FONT_16_ENABLED__

#if (1 == __FONT_20_ENABLED__ )
	#include "font20.h"
#endif // __FONT_20_ENABLED__

#if (1 == __FONT_24_ENABLED__ )
	#include "font24.h"
#endif // __FONT_24_ENABLED__

*/

#endif // __FONTS_H__
 

