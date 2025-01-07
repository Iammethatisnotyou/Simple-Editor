/* 
 * See LICENSE file for copyright and license details. 
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ncurses.h>
#include <stdlib.h>
#include <csignal>
#include <sstream>
using namespace std;

bool signal_received = false;
void control_c(int sig){
	signal_received = true;
}
void printing(const vector<string> raw_text){
	for(size_t i = 0; i < raw_text.size(); i++){
		string line_with_number = to_string(i+1) + ' ' + raw_text[i];
		mvprintw(i, 0, "%s", line_with_number.c_str());
	}
	clrtoeol(); /* Clear the rest of the line */
}
string reading(char *argv[], vector<string>& raw_text){
	string text;
	string elagent_text;
	ifstream File(argv[1]);

	while (getline (File, text)) {
		raw_text.push_back(text);}
	for(size_t i = 0; i < raw_text.size(); i++){
		elagent_text += raw_text[i] + '\n';
	}
	File.close();
	return elagent_text;
}
void saving(const vector<string> raw_text, char *argv[]){
	string new_text;
	ofstream File(argv[1]);
	for(size_t i = 0; i < raw_text.size(); i++){
		new_text.append(raw_text[i] + "\n");} 
	File << new_text;
	File.close();
}
void editing(char *argv[]){
	MEVENT event;
	int char_in_line = 0;
	int line_number  = 0;
	vector<string> raw_text;

	const string text = reading(argv, raw_text); /* This makes the screen not blank before input */
	printw(text.c_str()); /* Show text before input */
	while (true){
		if(signal_received){ /*Ctrl C*/
			signal_received = false;
			clear();
			printw("To exit press ESC to see if you want to save\nPress any key to continue");
			refresh();
			getch();
			flushinp(); /* Flush the input buffer to avoid leftover keys */
			clear();
			printing(raw_text);
			refresh();
			continue;}
		const int ch = getch();
		if(ch == 27){
			break;}
		else if(ch == KEY_MOUSE){
			if(getmouse(&event) == OK){
				if(event.y >= 0 && event.y <= raw_text.size()){
					line_number = event.y;}
				else if(event.y >= raw_text.size()){
					line_number = raw_text.size() -1;
				}
				else{
					line_number = 0;
				}

				if(event.x <= 0){
					char_in_line = 0;
				}
				else if(line_number >= 0 &&  line_number < raw_text.size()){
					char_in_line = min(event.x, (int)raw_text[line_number].size() );
				}
				else{
					char_in_line = 0;
				}
			}
		}
		else if(ch == ' '){
			raw_text[line_number].insert(char_in_line, +1, ' ');
			char_in_line++;}
		else if(ch == 8 /*8 = Ctrl Backspace*/){
			if(char_in_line > 0){
				int target_word_index = 2;
				int current_index = 0;
				string word;
				string target_word;
				istringstream last_word(raw_text[line_number]);
				while(last_word >> word){
					if(current_index == target_word_index){
						target_word = word;
						break;}
					current_index++;}
				if (!target_word.empty() && char_in_line - 1 >= 0) {
					raw_text[line_number].erase(char_in_line -1, target_word.length());} }

			else if(char_in_line == 0){
				char_in_line = raw_text[line_number -1].size();
				raw_text[line_number -1] += raw_text[line_number];
				raw_text.erase(raw_text.begin() + line_number);
				line_number--;} }
		else if(ch == '\n'){
			if(line_number < raw_text.size()){
				string new_line = raw_text[line_number].substr(char_in_line);
				raw_text[line_number] = raw_text[line_number].substr(0, char_in_line);
				raw_text.insert(raw_text.begin() + line_number +1, new_line);
				line_number++;
				char_in_line = 0;} }
		else if(ch == KEY_UP){
			if(raw_text.size() > 0){
				if(char_in_line > raw_text[line_number -1].size()){
					char_in_line = raw_text[line_number -1].size();}
				line_number--;} }
		else if(ch == KEY_DOWN){
			if(line_number +1 < raw_text.size()){
				if(char_in_line > raw_text[line_number +1].size()){
					char_in_line = raw_text[line_number +1].size();}
				line_number++;} }
		else if(ch == KEY_RIGHT){
			if(char_in_line < raw_text[line_number].size()){
				char_in_line++;} }
		else if(ch == KEY_LEFT){
			if(char_in_line > 0){
				char_in_line--;} }
		else if(ch == KEY_BACKSPACE || ch == 127){
			if(char_in_line > 0){
				raw_text[line_number].erase(char_in_line -1, 1);
				char_in_line--;}
			else if(line_number > 0){
				char_in_line = raw_text[line_number -1].size();
				raw_text[line_number -1] += raw_text[line_number];
				raw_text.erase(raw_text.begin() + line_number);
				line_number--;} }
		else if(isprint(ch)){
			if(line_number >= 0 && line_number < raw_text.size()){
				raw_text[line_number].insert(char_in_line, 1, ch);
				char_in_line++;} }
		clear();
		refresh();
		printing(raw_text);
		clrtoeol(); /* Clear the rest of the line */
		refresh();
	}
	clear();

	printw("Would you like to save? y or n");
	refresh();
	const char save_choice = getch();
	if(tolower(save_choice == 'y')){
		saving(raw_text, argv);
		printw("Saving to file");}
	else{
		printw("Not saving");}
	refresh();
	endwin();
	mousemask(0, NULL);
}
bool verification(int argc, char *argv[]){
	if(argc >= 3){
		cerr << "One too many arguments\n";
		return false;}
	else if(argc == 1){
		cerr << "Not enough arguments which file to open?\n";
		return false;}

	ifstream File(argv[1]);
	if(!File.is_open()){
		cerr << "couldn't find or open file\n";
		return false;
	}
	File.close();
	return true;
}

int main(int argc, char *argv[]){
	if (!verification(argc, argv)){
		return 1;}

	initscr();
	raw();
	mousemask(ALL_MOUSE_EVENTS, NULL); /* The old events mask, The events you want to listen to */
	keypad(stdscr, TRUE);
	noecho();
	cbreak();
	signal(SIGINT, control_c); /* Makes Control C not exit */

	editing(argv);
	return 0;
}
