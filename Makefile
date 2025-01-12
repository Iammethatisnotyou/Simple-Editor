# My text editor - A simple text editor in C++
# For information check the README, .gitignore and LICENSE files

NCURSES  = -lncurses
CXX      = g++
Warnings = -Wall -Wextra
CXXFLAGS = -std=c++17

TARGET  = se
SOURCES = se.cpp
HEADERS = config.h

MANPREFIX = $(PREFIX)/share/man
MANPAGE = se.1
PREFIX = /usr/local

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(NCURSES) -o $@ $(SOURCES)

clean:
	rm -f $(TARGET)

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARGET) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARGET)

	mkdir -p $(MANPREFIX)/man1
	cp $(MANPAGE) $(MANPREFIX)/man1/$(MANPAGE)
	chmod 644 $(MANPREFIX)/man1/$(MANPAGE)
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARGET)
	rm -f $(MANPREFIX)/man1/$(MANPAGE)
