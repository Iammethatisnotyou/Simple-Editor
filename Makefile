# My text editor - A simple text editor in C++
# For information check the README, .gitignore and LICENSE files

NCURSES  = -lncurses
CXX      = g++
Warnings = -Wall -Wextra
CXXFLAGS = -std=c++17

TARGET  = se
SOURCES = text.cpp
HEADERS = config.h
BACKUP  = bk_text.cpp

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(NCURSES) -o $@ $(SOURCES)
copy:
	cp $(SOURCES) $(BACKUP)
clean:
	rm -f $(TARGET)

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARGET) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARGET)
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARGET)
