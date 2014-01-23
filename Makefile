#!/usr/bin/make -f
# Copyright (C) 2013  Roel Janssen <roel@moefel.org>
#
# This file is part of InklingReader 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

SHELL           = /bin/sh

CC              = gcc
VPATH           = src:src/gui:src/converters:src/parsers:src/optimizers
LDLIBS          = `pkg-config gtk+-3.0 glib-2.0 librsvg-2.0 cairo --libs`
LDFLAGS         = `pkg-config gtk+-3.0 glib-2.0 librsvg-2.0 cairo --cflags`
#CFLAGS          = -Wall -O0 -g3 -DGTK_DISABLE_DEPRECATED=1 $(LDFLAGS)
CFLAGS          = -Wall -Os -DNDEBUG -DGTK_DISABLE_DEPRECATED=1 $(LDFLAGS)
OBJECTS         = main.o mainwindow.o mainwindow_sig.o svg.o png.o pdf.o wpi.o \
		  straight_lines.o
NAME            = InklingReader

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(LDLIBS) $(CFLAGS) $(OBJECTS) -o $(NAME)

.PHONY: clean
clean:
	@$(RM) -rf $(OBJECTS)

.PHONY: clean-all
clean-all:
	@$(MAKE) clean
	@rm -rf *~ */*~ */*/*~ */*/*/*~

.PHONY: module-info
module-info:
	@printf "%-25s %s\r\n" "MODULE NAME" "SIZE"; \
	echo "------------------------  ----------"; \
	ls -lS | egrep "\.o$$|$(NAME)$$" | awk '{printf "%-25s %6.2f K\r\n", $$9, $$5 / 1000}'

## To compile the application for Windows we need to pass all source files at 
## once to avoid problems with the linker.
.PHONY: win32
win32:
	$(CC) $(LBLIBS) src/main.c src/gui/mainwindow.c src/gui/mainwindow_sig.c \
	src/parsers/wpi.c src/converters/svg.c src/converters/png.c src/converters/pdf.c \
	src/optimizers/straight_lines.c $(CFLAGS) -o $(NAME)
