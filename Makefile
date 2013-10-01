CC	= gcc
TARGET	= build/InklingReader
OPT	= -Wall -g -O2
GTKLIBS	= `pkg-config --cflags --libs gtk+-3.0` -DGTK_DISABLE_DEPRECATED=1
GLIBS   = `pkg-config --cflags --libs glib-2.0` -DGTK_DISABLE_DEPRECATED=1
LIBS	=
OBJECTS	= gui_mainwindow.o co_svg.o parsers_wpi.o main.o

first: all

parsers_wpi.o: src/parsers/wpi.h src/parsers/wpi.c
	$(CC) $(OPT) $(LIBS) $(GLIBS) -c src/parsers/wpi.c -o parsers_wpi.o

co_svg.o: src/converters/svg.h src/converters/svg.c
	$(CC) $(OPT) $(LIBS) $(GLIBS) -c src/converters/svg.c -o co_svg.o

main.o: src/main.c
	$(CC) $(GTKLIBS) $(OPT) $(LIBS) -c src/main.c -o main.o

gui_mainwindow.o: src/gui/mainwindow.h src/gui/mainwindow.c
	$(CC) $(OPT) $(GTKLIBS) -c src/gui/mainwindow.c -o gui_mainwindow.o

all: $(OBJECTS)
	$(CC) $(OPT) $(GTKLIBS) $(LIBS) $(OBJECTS) -o $(TARGET)

clean:
	rm -rf *.o
