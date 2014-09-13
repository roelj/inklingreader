# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS 		= $(gtk_CFLAGS) $(glib_CFLAGS) $(cairo_CFLAGS) $(rsvg_CFLAGS)
bin_PROGRAMS 		= inklingreader
inklingreader_SOURCES 	= src/main.c src/gui/mainwindow.c src/gui/mainwindow.h \
			  src/converters/svg.c src/converters/png.c src/converters/json.c \
			  src/converters/pdf.c src/parsers/wpi.c src/high/conversion.c \
			  src/datatypes/configuration.c src/datatypes/configuration.h \
			  src/optimizers/point-reduction.h src/optimizers/point-reduction.c
inklingreader_LDADD 	= $(gtk_LIBS) $(glib_LIBS) $(cairo_LIBS) $(rsvg_LIBS)

if OS_LINUX
inklingreader_LDADD    += -ldl -lpthread
else
inklingreader_LDADD    += -lshell32
inklingreader_LDFLAGS   = -mwindows
endif

if ENABLE_GCOV_OPTION
AM_CFLAGS	       += -fprofile-arcs -ftest-coverage
endif

docs:
	@cd doc/; texi2pdf inklingreader.texi 

docs-clean:
	@cd doc/; ${RM} -rf *.aux *.ky *.pg *.tp *.cp *.log *.vr *.fn *.toc

docs-doxygen:
	@cd doc/; doxygen Doxyfile

gcov-clean:
	@rm -rf src/*.gcno src/*.gcda src/*/*.gcno src/*/*.gcda

.PHONY: docs docs-clean docs-doxygen gcov-clean
