NCURSES  = -lncurses
CXX      = g++
CXXFLAGS = -std=c++17 -O2
WARNINGS = -Wall -Wextra -Werror -Wpedantic -Wshadow

TARGET  = se
MAIN 	= src/se.cpp
SOURCE  = src/util.c
HEADERS = src/config.h

MANPREFIX = $(PREFIX)/share/man
MANPAGE   = documentation/se.1
PREFIX	  = /usr/local
