# Simple Editor - a simple text editor

include config.mk

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(NCURSES) -o $@ $(SOURCE) $(MAIN)

build: clean $(TARGET)

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
