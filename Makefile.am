AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS 		= $(INKLINGREADER_CFLAGS)
bin_PROGRAMS 		= inklingreader
inklingreader_SOURCES 	= src/main.c src/gui/mainwindow.c src/gui/mainwindow.h \
			  src/converters/svg.c src/converters/png.c src/converters/json.c \
			  src/converters/pdf.c src/parsers/wpi.c src/high/conversion.c \
			  src/datatypes/configuration.c src/datatypes/configuration.h
inklingreader_LDADD 	= $(INKLINGREADER_LIBS)

docs:
	@cd doc/; texi2pdf inklingreader.texi 

docs-clean:
	@cd doc/; ${RM} -rf *.aux *.ky *.pg *.tp *.cp *.log *.vr *.fn *.toc

.PHONY: docs docs-clean
