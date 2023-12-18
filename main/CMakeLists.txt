set	( srcs	"pretty_effect.c"
		"main.c"
		"decode_image.c"
	)

idf_component_register	( 	SRCS ${srcs}
				INCLUDE_DIRS "." "st_2_inch"
				
				EMBED_FILES image.jpg
			)
