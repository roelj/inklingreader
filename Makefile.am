AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS 		= $(INKLINGREADER_CFLAGS)
bin_PROGRAMS 		= inklingreader
inklingreader_SOURCES 	= src/main.c src/gui/mainwindow.c src/gui/mainwindow.h \
			  src/converters/svg.c src/converters/png.c src/converters/json.c \
			  src/converters/pdf.c src/parsers/wpi.c src/high/conversion.c \
			  src/datatypes/configuration.c src/datatypes/configuration.h
inklingreader_LDADD 	= $(INKLINGREADER_LIBS)
