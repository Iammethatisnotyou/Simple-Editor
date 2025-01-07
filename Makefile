# My text editor - A simple text editor in C++
# For information check the README, .gitignore and LICENSE files

NCURSES = -lncurses
CXX = g++
Warnings = -Wall -Wextra
CXXFLAGS = -std=c++17

TARGET = text_editor

all: $(TARGET)

$(TARGET): text.cpp
	$(CXX) $(CXXFLAGS) $(Warnings) $(NCURSES) -o $@ text.cpp
copy:
	cp text.cpp bk_text.cpp
clean:
	rm -f $(TARGET)

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(TARGET) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(TARGET)
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/$(TARGET)
