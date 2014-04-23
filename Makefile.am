AM_CFLAGS 		= $(INKLINGREADER_CFLAGS)
VPATH   		= src:src/gui:src/converters:src/parsers:src/high
bin_PROGRAMS 		= inklingreader
inklingreader_SOURCES 	= main.c mainwindow.c mainwindow_sig.c svg.c png.c json.c pdf.c wpi.c conversion.c configuration.c
inklingreader_LDADD 	= $(INKLINGREADER_LIBS)

.PHONY: module-info
module-info:
	@printf "%-25s %s\r\n" "MODULE NAME" "SIZE"; \
	echo "------------------------  ----------"; \
	ls -lS | egrep "\.o$$|$(bin_PROGRAMS)$$" | awk '{printf "%-25s %6.2f K\r\n", $$9, $$5 / 1000}'
