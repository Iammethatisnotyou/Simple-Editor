/*
 * See LICENSE file for copyright and license details.
 */
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <thread>
#include <vector>

#include "config.h"
#include "util.h"

using namespace std;

void die(const char *msg);
void malloc_check(void *ptr);
void *ecalloc(size_t nmemb, size_t size);

bool signal_received = false;
void control_c(int) { signal_received = true; }

void auto_saving(char **argv, vector<string> &raw_text);
void editing(char **argv);
void printing(const vector<string> raw_text, unsigned int &line_number, unsigned int &char_in_line, char **argv);
string reading(char **argv, vector<string> &raw_text);
void saving(const vector<string> raw_text, char **argv);
size_t storage_size(const char *argv);
void verification(int argc, char **argv);

size_t storage_size(const char *argv) {
	struct stat st;
	if (stat(argv, &st) != 0) {
		return 0;
	}
	return st.st_size;
}
void printing(const vector<string> raw_text, unsigned int &line_number, unsigned int &char_in_line, char **argv) {
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	string percent_through = to_string((line_number * 100) / (raw_text.size() - 1)) + "%";
	string bottom_bar      = "\n" + string(argv[1]) + " " + percent_through + " " + to_string(line_number + 1) + ":" + to_string(char_in_line + 1);
	string line_without_number;
	for (size_t i = 0; i < raw_text.size(); i++) {
		line_without_number     = raw_text[i];
		string line_with_number = to_string(i + 1) + ' ' + raw_text[i];
		if (i == line_number) {
			if (numbered_lines) {
				line_with_number.insert(char_in_line + to_string(i + 1).size() + 1, "|");
			} else {
				line_without_number.insert(char_in_line, "|");
			}
		}
		if (numbered_lines) {
			mvprintw(i, 0, "%s", line_with_number.c_str());
		} else {
			mvprintw(i, 0, "%s", line_without_number.c_str());
		}
		if (w.ws_row > raw_text.size()) {
			size_t lines_to_print = (w.ws_row - 1) - (raw_text.size());
			for (size_t j = 0; j < lines_to_print; j++) {
				printw("\n%c", empty_line_char);
			}
		}
		printw("%s", bottom_bar.c_str());
	}
}
string reading(char **argv, vector<string> &raw_text) {
	string text;
	string elagent_text;
	ifstream File(argv[1]);

	while (getline(File, text)) {
		raw_text.push_back(text);
	}
	for (size_t i = 0; i < raw_text.size(); i++) {
		elagent_text += raw_text[i] + '\n';
	}
	File.close();
	return elagent_text;
}
void saving(const vector<string> raw_text, char **argv) {
	string new_text;
	ofstream File(argv[1]);
	for (size_t i = 0; i < raw_text.size(); i++) {
		new_text.append(raw_text[i] + "\n");
	}
	File << new_text;
	File.close();
}
void auto_saving(char **argv, vector<string> &raw_text) {
	if (!auto_save) {
		return;
	}
	while (true) {
		this_thread::sleep_for(chrono::seconds(auto_save_interval));
		saving(raw_text, argv);
	}
}
void editing(char **argv) {
	MEVENT event;
	mousemask(ALL_MOUSE_EVENTS, NULL); // Or just BUTTON1_PRESSED for left-click
	unsigned int char_in_line = 0, line_number = 0;
	vector<string> raw_text;

	thread t1(auto_saving, argv, ref(raw_text));
	t1.detach();

	const string text = reading(argv, raw_text); /* This makes the screen not blank before input */
	if (raw_text.empty()) {
		raw_text.push_back(" ");
	}                                                    /* If a file is empty can't edit it */
	printing(raw_text, line_number, char_in_line, argv); /* Show text before input */
	while (true) {
		if (signal_received) { /*Ctrl C*/
			signal_received = false;
			clear();
			printw("To exit press ESC to see if you want to save\nPress any key to continue");
			refresh();
			getch();
			flushinp(); /* Flush the input buffer to avoid leftover keys */
			clear();
			printing(raw_text, line_number, char_in_line, argv);
			refresh();
			continue;
		}
		const int ch = getch();
		if (ch == 27) {
			break;
		}

		else if (ch == 9) {
			if (tab_spaces) {
				raw_text[line_number].insert(char_in_line, tab_width, ' ');
				char_in_line += tab_width;
			} else {
				raw_text[line_number].insert(char_in_line, +1, '\t');
				char_in_line++;
			}
		} else if (ch == KEY_MOUSE) {
			if (getmouse(&event) == OK) {
				if (event.bstate & BUTTON1_CLICKED) {
					if (event.y >= 0 && static_cast<size_t> (event.y) <= raw_text.size()) {
						line_number = event.y;
					} else if (static_cast<size_t> (event.y) >= raw_text.size()) {
						line_number = raw_text.size() - 1;
					} else {
						line_number = 0;
					}

					if (event.x <= 0) {
						char_in_line = 0;
					} else if (line_number > 0 && line_number < raw_text.size()) {
						char_in_line = min(event.x, (int)raw_text[line_number].size());
					} else {
						char_in_line = 0;
					}
				}
			}
		} else if (ch == ' ') {
			raw_text[line_number].insert(char_in_line, +1, ' ');
			char_in_line++;
		} else if (ch == 8 /*8 = Ctrl Backspace*/) {
			if (char_in_line > 0) {
				int target_word_index = 2;
				int current_index     = 0;
				string word;
				string target_word;
				istringstream last_word(raw_text[line_number]);
				while (last_word >> word) {
					if (current_index == target_word_index) {
						target_word = word;
						break;
					}
					current_index++;
				}
				if (!target_word.empty() && char_in_line - 1 > 0) {
					raw_text[line_number].erase(char_in_line - 1, target_word.length());
				}
			}

			else if (char_in_line == 0) {
				char_in_line               = raw_text[line_number - 1].size();
				raw_text[line_number - 1] += raw_text[line_number];
				raw_text.erase(raw_text.begin() + line_number);
				line_number--;
			}
		} else if (ch == '\n') {
			if (line_number < raw_text.size()) {
				string new_line       = raw_text[line_number].substr(char_in_line);
				raw_text[line_number] = raw_text[line_number].substr(0, char_in_line);
				raw_text.insert(raw_text.begin() + line_number + 1, new_line);
				char_in_line = 0;
				line_number++;
			}
		} else if (ch == KEY_UP) {
			if (raw_text.size() > 0 && line_number > 0) {
				if (char_in_line > raw_text[line_number - 1].size()) {
					char_in_line = raw_text[line_number - 1].size();
				}
				line_number--;
			}
		} else if (ch == KEY_DOWN) {
			if (line_number + 1 < raw_text.size()) {
				if (char_in_line > raw_text[line_number + 1].size()) {
					char_in_line = raw_text[line_number + 1].size();
				}
				line_number++;
			}
		} else if (ch == KEY_RIGHT) {
			if (char_in_line < raw_text[line_number].size()) {
				char_in_line++;
			} else if (horizontal_line_wrap == true && line_number < raw_text.size() - 1) {
				char_in_line = 0;
				line_number++;
			}
		} else if (ch == KEY_LEFT) {
			if (char_in_line > 0) {
				char_in_line--;
			} else if (horizontal_line_wrap == true && line_number > 0) {
				char_in_line = raw_text[line_number - 1].size();
				line_number--;
			}
		} else if (ch == KEY_BACKSPACE || ch == 127) {
			if (char_in_line > 0) {
				raw_text[line_number].erase(char_in_line - 1, 1);
				char_in_line--;
			} else if (line_number > 0) {
				char_in_line               = raw_text[line_number - 1].size();
				raw_text[line_number - 1] += raw_text[line_number];
				raw_text.erase(raw_text.begin() + line_number);
				line_number--;
			}
		} else if (isprint(ch)) {
			if (line_number < raw_text.size()) {
				raw_text[line_number].insert(char_in_line, 1, ch);
				char_in_line++;
			}
		}
		clear();
		refresh();
		printing(raw_text, line_number, char_in_line, argv);
		refresh();
	}
	clear();

	printw("Would you like to save? y or n");
	refresh();
	const char save_choice = getch();
	if (tolower(save_choice == 'y')) {
		string format = "bytes";
		saving(raw_text, argv);
		size_t file_size = storage_size(argv[1]);

		if (file_size >= 1024) {
			file_size /= 1024;
			format     = "kilobytes";
		}
		string file_size_format = to_string(file_size) + " " + format;
		printw("\nSaved to file\n");
		printw("%s\n", file_size_format.c_str());
		printw("\nPress any key to exit");
		getch();
	} else {
		printw("\nNot saving\n");
	}
	endwin();
	mousemask(0, NULL);
}
void verification(int argc, char **argv) {
	if (argc >= 3) {
		die("One too many arguments:");
	} else if (argc == 1) {
		die("Not enough arguments which file to open?:");
	}

	ifstream File(argv[1]);
	if (!File.is_open()) {
		cerr << "couldn't find or open file\n";
		exit(1);
	}
	File.close();
}
int main(int argc, char **argv) {
	verification(argc, argv);

	initscr();
	raw();
	mousemask(ALL_MOUSE_EVENTS, NULL); /* The old events mask, The events you want to listen to */
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	curs_set(0);
	signal(SIGINT, control_c); /* Makes Control C not exit */
	editing(argv);
	return 0;
}
