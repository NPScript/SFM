#include <boost/filesystem.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/file_status.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <curses.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>

#include "config.hpp"

class Sfm {
	private:
		WINDOW * windows[3];
		WINDOW * shell_panel;
		WINDOW * file_view;
		std::vector<std::string> files[3];
		int screen_size[2];
		boost::filesystem::path current_path;
		int selection;
		bool show_hidden;
		bool shell_command;
		bool can_enter;
	public:
		Sfm();
		~Sfm();

		void refresh_display();
		void refresh_files();
		void mainloop();
		void insert_files(const int &, const std::string &);
		void run_command(const std::string & = "");
};

Sfm::Sfm() : current_path(boost::filesystem::current_path()), show_hidden(false), shell_command(false), selection(0) {
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	noecho();
	curs_set(0);

	for (int i = 0; i < ITEM_COLORS.size(); ++i) {
		init_pair(i, ITEM_COLORS[i].first, ITEM_COLORS[i].second);
	}

	getmaxyx(stdscr, screen_size[0], screen_size[1]);
	--screen_size[0];

	for (int i = 0; i < 3; ++i) {
		windows[i] = newwin(screen_size[0], screen_size[1] / 3, 0, screen_size[1] / 3 * i);
	}

	shell_panel = newwin(1, screen_size[1], screen_size[0], 0);
	file_view = newwin(1, 1, 0, 0);

	this->refresh_files();
	this->refresh_display();
}

Sfm::~Sfm() {
	echo();
	curs_set(1);
	for (int i = 0; i < 3; ++i) {
		delwin(windows[i]);
	}

	delwin(shell_panel);
	delwin(file_view);

	clear();
	refresh();
	endwin();
}

void Sfm::mainloop() {
	char ch;

	while ((ch = getch()) != NAV_KEYS["quit"]) {
		if (ch == NAV_KEYS["left"] && current_path != boost::filesystem::path("/")) {
			selection = std::distance(this->files[0].begin(), std::find(this->files[0].begin(), this->files[0].end(), this->current_path.filename().string()));
			this->current_path = this->current_path.parent_path();
		} else if (ch == NAV_KEYS["right"]) {
			std::string next_path = (this->current_path.string() == "/" ? "" : this->current_path.string()) + "/" + this->files[1][selection];
			if (this->can_enter) {
				this->current_path = next_path;
				selection = 0;
			}
		} else if (ch == NAV_KEYS["down"]) {
			if (selection < files[1].size() - 1)
				++selection;
		}
		else if (ch == NAV_KEYS["up"]) {
			if (selection > 0)
				--selection;
		} else if (ch == NAV_KEYS["toggle_hidden"]) {
			show_hidden = !show_hidden;
		} else if (ch == NAV_KEYS["open"] || ch == '\n') {
			std::string next_path = (this->current_path.string() == "/" ? "" : this->current_path.string()) + "/" + this->files[1][selection];
			if (boost::filesystem::is_directory(next_path)) {
				this->current_path = next_path;
				selection = 0;
			} else {
				system((std::string("xdg-open ") + next_path).c_str());
			}
		} else if (ch == NAV_KEYS["execute"]) {
			run_command();
		} else {
			for (std::pair<const char, std::string> & i : SHORT_CUTS) {
				if (ch == i.first) {
					run_command(i.second);
					break;
				}
			}
		}

		this->refresh_files();
		this->refresh_display();
	}
}

void Sfm::refresh_display() {
	getmaxyx(stdscr, screen_size[0], screen_size[1]);
	--screen_size[0];
	clear();
	refresh();

	for (int i = 0; i < 3; ++i) {
		wresize(windows[i], screen_size[0], screen_size[1] / 3);
		mvwin(windows[i], 0, screen_size[1] / 3 * i);

		wclear(windows[i]);

		for (int j = 0; j < files[i].size(); ++j) {
			bool is_active = this->files[1][selection] == this->files[i][j] && i == 1;
			
			if (is_active) {
				wcolor_set(windows[1], 2, 0);
			}

			if (files[i][j].size() > screen_size[1] / 3 - 2) {
				std::string str = files[i][j];
				
				int dot_pos = str.find_last_of(".");

				if (dot_pos == std::string::npos || dot_pos == 0) {
					dot_pos = str.size();
				}
				
				str = str.substr(0, screen_size[1] / 3 - 3 - str.size() + dot_pos) + "…" + str.substr(dot_pos, std::string::npos);

				mvwprintw(windows[i], j + 1, 1, str.c_str());
			} else {
				mvwprintw(windows[i], j + 1, 1, (files[i][j] + std::string(screen_size[1] / 3 - 2 - files[i][j].size(), ' ')).c_str());
			}

			if (is_active) {
				wcolor_set(windows[1], 0, 0);
			}
		}

		wcolor_set(windows[i], 1, 0);
		box(windows[i], 0, 0);
		wcolor_set(windows[i], 0, 0);
		wrefresh(windows[i]);
	}

	if (!boost::filesystem::is_directory(current_path.string() + "/" + files[1][selection])) {

		wclear(file_view);

		mvwin(file_view, 1, screen_size[1] / 3 * 2 + 1);
		wresize(file_view, screen_size[0] - 2, screen_size[1] / 3 - 2);

		std::ifstream f(current_path.string() + "/" + files[1][selection]);
		std::string line;
		std::string view("");

		if (f.bad()) {
			wcolor_set(file_view, 1, 0);
			view = "Bad File";
		} else {
			wcolor_set(file_view, 0, 0);
			for (int i = 0; i < 50 && std::getline(f, line); ++i) {
				view += line + "\n";
			}

			if (view.empty()) {
				wcolor_set(file_view, 1, 0);
				view = "Empty File";
			}
		}

		f.close();

		mvwprintw(file_view, 0, 0, view.c_str());

		wrefresh(file_view);
		wrefresh(windows[2]);
	}

	mvwin(shell_panel, screen_size[0], 0);
	wclear(shell_panel);
	if (current_path.string().size() < screen_size[1]) {
		mvwprintw(shell_panel, 0, 0, current_path.c_str());
	} else {
		mvwprintw(shell_panel, 0, 0, ("…" + current_path.string().substr(current_path.string().size() - screen_size[1] + 1, std::string::npos)).c_str());
	}
	wrefresh(shell_panel);

}

void Sfm::refresh_files() {
	this->files[0].clear();
	this->files[1].clear();
	this->files[2].clear();

	if (current_path != boost::filesystem::path("/")) {
		insert_files(0, current_path.parent_path().string());
	} else {
		this->files[0].push_back("/");
	}

	insert_files(1, current_path.string());

	boost::filesystem::path active(current_path.string() + "/" + this->files[1][selection]);
	
	if (boost::filesystem::is_directory(active)) {
		try {
			insert_files(2, active.string());
			this->can_enter = true;
			if (this->files[2].empty()) {
				this->files[2].push_back("empty");
				this->can_enter = false;
			}
		} catch (boost::filesystem::filesystem_error & e) {
			this->files[2].push_back(e.code().message());
			this->can_enter = false;
		}
	} else {
		this->can_enter = false;
	}
}

void Sfm::insert_files(const int & id, const std::string & path) {
	for (boost::filesystem::directory_entry & i : boost::filesystem::directory_iterator(path)) {
		if (!(i.path().filename().string()[0] == '.' && !show_hidden)) {
			this->files[id].push_back(i.path().filename().string());
		}
	}

	std::sort(this->files[id].begin(), this->files[id].end());
}

void Sfm::run_command(const std::string & cmd) {
	std::string command = cmd;
	curs_set(1);

	size_t sel = command.find("\?");
	if (sel != std::string::npos) {
		command.replace(sel, 1, this->files[1][selection]);
	}

	wclear(shell_panel);
	wprintw(shell_panel, (":" + command).c_str());
	wrefresh(shell_panel);
	wmove(shell_panel, 0, command.size() + 1);


	char ch;
	if (command.back() == '\n') {
		command.pop_back();
		ch = '\n';
	} else {
		while ((ch = getch()) != 27) {
			if (ch == '\n') {
				break;
			} else if (ch == 127) {
				if (!command.empty())
					command.pop_back();
			} else {
				command += ch;
			}

			wclear(shell_panel);
			wprintw(shell_panel, (":" + command).c_str());
			wrefresh(shell_panel);
			wmove(shell_panel, 0, command.size() + 1);
		}
	}

	if (ch == '\n') {
		command = "(cd " + current_path.string() + " && " + command + ")";
		system(command.c_str());
		
	}
	curs_set(0);
	cbreak();
}

int main(int argc, char ** argv) {
	Sfm sfm;
	sfm.mainloop();
	return 0;
}
