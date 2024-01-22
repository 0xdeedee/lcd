set	( srcs	"lcd_driver.c"
		"main.c"
		"gui_paint.c"
	)

idf_component_register	( 	SRCS ${srcs}
				INCLUDE_DIRS "." "st_2_inch"
				
				EMBED_FILES image.jpg
			)
