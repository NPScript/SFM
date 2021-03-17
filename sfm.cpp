#include <boost/filesystem.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/file_status.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <sys/wait.h>
#include <curses.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <magic.h>

void start_session();
void end_session();

void quit();
void open();
void toggle_hidden();
void move();
void refresh_contents();
void refresh_dimensions();
void refresh_files();
void mainloop();
void insert_files(const int &, const std::string &);
void run_command();
void refresh_size();
std::string text_preview();
std::string img_preview(const std::string &);
std::string preview_file();
std::string preview_pdf();

struct arg {
	int v;
	int h;
	std::string cmd;
} cmd_arg;

#include "config.hpp"

WINDOW * windows[3];
WINDOW * shell_panel;
WINDOW * file_view;
std::vector<std::string> files[3];
boost::filesystem::path current_path;
int screen_size[2];
int selection;
bool show_hidden;
bool shell_command;
bool can_enter;
bool running;

void refresh_size() {
	getmaxyx(stdscr, screen_size[0], screen_size[1]);
	--screen_size[0];
}


void start_session() {
	show_hidden = false;
	shell_command = false;
	selection = 0;
	running = true;

	setlocale(LC_ALL, "");

	refresh_files();

	initscr();
	start_color();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED, NULL);
	mouseinterval(0);

	for (int i = 0; i < ITEM_COLORS.size(); ++i) {
		init_pair(i, ITEM_COLORS[i].first, ITEM_COLORS[i].second);
	}

	refresh_size();

	for (int i = 0; i < 3; ++i) {
		windows[i] = newwin(screen_size[0], screen_size[1] / 3, 0, screen_size[1] / 3 * i);
	}

	shell_panel = newwin(1, screen_size[1], screen_size[0], 0);
	file_view = newwin(1, 1, 0, 0);

	refresh_contents();
}

void end_session() {
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

void move() {
		if (cmd_arg.v < 0) {
			for (; cmd_arg.v < 0 && current_path != boost::filesystem::path("/"); ++cmd_arg.v) {
				selection = std::distance(files[0].begin(), std::find(files[0].begin(), files[0].end(), current_path.filename().string()));
				current_path = current_path.parent_path();
			}

		} else if (cmd_arg.v > 0) {
			
			for (; cmd_arg.v > 0 && can_enter; --cmd_arg.v) {
				std::string next_path = (current_path.string() == "/" ? "" : current_path.string()) + "/" + files[1][selection];
				if (can_enter) {
					current_path = next_path;
					selection = 0;
				}

				refresh_files();
			}
		}

		if (cmd_arg.cmd == "top") {
			selection = 0;
		} else if (cmd_arg.cmd == "bottom") {
			selection = files[1].size() - 1;
		} else {
			selection += cmd_arg.h;
		}

		if (selection < 0)
			selection = 0;
		else if (selection > files[1].size() - 1)
			selection = files[1].size() - 1;
}

void toggle_hidden() {
	show_hidden = !show_hidden;
}

void open() {
	std::string next_path = (current_path.string() == "/" ? "" : current_path.string()) + "/" + files[1][selection];
	if (boost::filesystem::is_directory(next_path)) {
		current_path = next_path;
		selection = 0;
	} else {
		std::string prg = DEF_PROGS["fallback"].first;
		std::string ext = boost::filesystem::path(next_path).extension().string();
		int parallelize = DEF_PROGS["fallback"].second;
		
		if (DEF_PROGS.find(ext) != DEF_PROGS.end()) {
			prg = DEF_PROGS[ext].first;
			parallelize = DEF_PROGS[ext].second;
		}

		endwin();

		if (parallelize) {
			pid_t stat = fork();

			if (stat == 0) {
				system(("setsid " + prg + " \"" + next_path + "\"").c_str());
				exit(0);
			} else {
				wait(&stat);
				initscr();
				curs_set(0);
				noecho();
			}
		} else {
			system((prg + " \"" + next_path + "\"").c_str());
			initscr();
			curs_set(0);
			noecho();
		}
	}
}

void quit() {
	running = false;
}

void mainloop() {
	unsigned ch;
	bool selected = false;
	MEVENT event;

	while (running) {
		ch = getch();
		if (NAV_KEYS.find(ch) != NAV_KEYS.end()) {
			cmd_arg.cmd = NAV_KEYS.find(ch)->second.second.cmd;
			cmd_arg.h = NAV_KEYS.find(ch)->second.second.h;
			cmd_arg.v = NAV_KEYS.find(ch)->second.second.v;
			(*NAV_KEYS.find(ch)->second.first)();
			if (!running)
				break;
			refresh_files();
			refresh_contents();
		} else if (SHORT_CUTS.find(ch) != SHORT_CUTS.end()) {
			cmd_arg.cmd = SHORT_CUTS.find(ch)->second;
			run_command();
			refresh_files();
			refresh_contents();
		} else if (ch == KEY_RESIZE) {
			refresh_dimensions();
			refresh_contents();
		} else if (getmouse(&event) == OK) {
			if (event.bstate & BUTTON1_PRESSED) {
				if (event.x < screen_size[1] / 3) {
					cmd_arg = {.v = -1};
					move();
				} else if (event.x < screen_size[1] * 2 / 3 && files[1].size() + 1 > event.y) {
					selection = event.y - 1;
					selected = true;
				} else if (event.x > screen_size[1] * 2 / 3 && event.x < screen_size[1]) {
					cmd_arg = {.v = 1};
					move();
				}
				
				refresh_files();
				refresh_contents();

			} else if (event.bstate & BUTTON1_RELEASED && selected) {
				selected = false;
				cmd_arg = {.v = 1};
				move();

				refresh_files();
				refresh_contents();
			}
		}
	}
}

std::string preview_file() {
	std::string view;
	magic_t handle = magic_open(MAGIC_NONE|MAGIC_COMPRESS);
	magic_load(handle, NULL);
	std::string type = magic_file(handle, (current_path.string() + "/" + files[1][selection]).c_str());

	if (type.find("text") != std::string::npos) {
		view = text_preview();
	} else if (type.find("image data") != std::string::npos) {
		view = img_preview(type);
	} else if (type.find("PDF") != std::string::npos) {
		view = preview_pdf();
		if (view.empty())
			return type + "\n\n Install poppler to prewview PDF";
	} else {
		return type;
	}

	return view;
}

std::string text_preview() {
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

		return view;
}

std::string img_preview(const std::string & type) {
	if (!boost::filesystem::exists("/usr/lib/w3m/w3mimgdisplay"))
		return type + "\n\nInstall w3m to display images";

	std::string image = current_path.string() + "/" + files[1][selection];

	size_t width;
	size_t height;
	size_t x;

	if (type.find("PNG image data") != std::string::npos) {
		size_t size_inf_begin = type.find_first_of(",");
		std::string size_str = type.substr(size_inf_begin + 2, type.substr(size_inf_begin + 2).find_first_of(","));
		width = std::stoi(size_str.substr(0, size_str.find_first_of(" ")));
		height = std::stoi(size_str.substr(size_str.find_last_of(" ")));
	} else if (type.find("JPEG image data") != std::string::npos) {
		size_t size_inf_end = type.find_last_of(",");
		size_t size_inf_begin = type.substr(0, size_inf_end).find_last_of(",") + 2;
		std::string size_str = type.substr(size_inf_begin, size_inf_end - size_inf_begin);

		width = std::stoi(size_str.substr(0, size_str.find("x")));
		height = std::stoi(size_str.substr(size_str.find("x") + 1));

	} else {
		return type;
	}

	height = height * ((screen_size[1] - 2) / 3.0f * 8 / (width - 10));
	width = (screen_size[1] - 2) / 3 * 8 - 10;
	x = screen_size[1] / 3.0f * 16 + 12;

	std::string cmd = "$(sleep 0.1 && printf '0;1;" + std::to_string(x) + ";18;" + std::to_string(width) + ";" + std::to_string(height) + ";";
	cmd += ";;;;";
	cmd += image;
	cmd += "\n4;\n3;\n'";
	cmd += " | /usr/lib/w3m/w3mimgdisplay) &";

	system(cmd.c_str());

	return "";
}

std::string preview_pdf() {
	std::string cmd = "pdftotext -f 1 '" + current_path.string() + "/" + files[1][selection] + "' -";
	FILE * ptt = popen(cmd.c_str(), "r");

	if (ptt == nullptr)
		return "";

	std::string text;
	char line[1024];

	while (fgets(line, 1024, ptt) != NULL)
		text += line;

	fclose(ptt);

	return text;
}

void refresh_dimensions() {
	refresh_size();
	endwin();
	initscr();

	for (int i = 0; i < 3; ++i) {
		wresize(windows[i], screen_size[0], screen_size[1] / 3);
		mvwin(windows[i], 0, screen_size[1] / 3 * i);
	}

	mvwin(shell_panel, screen_size[0], 0);
	wresize(shell_panel, 1, screen_size[1]);
}

void refresh_contents() {
	clear();
	refresh();

	for (int i = 0; i < 3; ++i) {

		wclear(windows[i]);

		for (int j = 0; j < screen_size[0] - 2; ++j) {
			unsigned scrl = 0;

			if (selection > screen_size[0] - 3 && i == 1) {
				scrl = selection - screen_size[0] + 3;
			}

			if (j - scrl < 0)
				continue;

			if (j + scrl == files[i].size())
				break;

			bool is_active = selection == j + scrl && i == 1;
			
			if (is_active) {
				wcolor_set(windows[1], 2, 0);
			}

			std::string str = files[i][j + scrl];
			size_t extra = str.size();

			try {
				std::string dir = (i == 0 ? current_path.parent_path().string() : "");
				dir += (i == 1 ? current_path.string() : "");
				dir += (i == 2 ? current_path.string() + "/" + files[1][selection] : "");
				dir += "/";

				if (boost::filesystem::is_directory(dir + files[i][j + scrl])) {
					str = " " + DIRECTORY_ICON + "  " + str;
				} else if (!ICONS_SUFFIX[boost::filesystem::path(current_path.string() + "/" + files[i][j + scrl]).extension().string()].empty()) {
					str = " " + ICONS_SUFFIX[boost::filesystem::path(current_path.string() + "/" + files[i][j + scrl]).extension().string()] + "  " + str;
				} else {
					str = " " + ICONS_SUFFIX[""] + "  " + str;
				}

			} catch (boost::filesystem::filesystem_error & e) {
				str = " ⚠  " + str;
			}

			extra = str.size() - extra - 3;

			if (str.size() - extra + 2 > screen_size[1] / 3 - 2) {
				
				int dot_pos = str.find_last_of(".");

				if (dot_pos == std::string::npos || dot_pos == 0) {
					dot_pos = str.size();
				}
				
				str = str.substr(0, screen_size[1] / 3 - 5 - str.size() + extra + dot_pos) + "…" + str.substr(dot_pos, std::string::npos) + " ";

				mvwprintw(windows[i], j + 1, 1, str.c_str());
			} else {
				mvwprintw(windows[i], j + 1, 1, (str + std::string(screen_size[1] / 3 - 3 - str.size() + extra, ' ') + " ").c_str());
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

		std::string view = preview_file();

		if (view.size()) {
			mvwprintw(file_view, 0, 0, view.c_str());

			wrefresh(file_view);
			wrefresh(windows[2]);
		}
	}

	wclear(shell_panel);
	if (current_path.string().size() < screen_size[1]) {
		mvwprintw(shell_panel, 0, 0, current_path.c_str());
	} else {
		mvwprintw(shell_panel, 0, 0, ("…" + current_path.string().substr(current_path.string().size() - screen_size[1] + 1, std::string::npos)).c_str());
	}
	wrefresh(shell_panel);

}

void refresh_files() {
	files[0].clear();
	files[1].clear();
	files[2].clear();

	if (current_path != boost::filesystem::path("/")) {
		insert_files(0, current_path.parent_path().string());
	} else {
		files[0].push_back("/");
	}

	insert_files(1, current_path.string());

	try {
		boost::filesystem::path active(current_path.string() + "/" + files[1][selection]);

		if (boost::filesystem::is_directory(active)) {
			insert_files(2, active.string());
			can_enter = true;
			if (files[2].empty()) {
				files[2].push_back("empty");
				can_enter = false;
			}
		} else {
			can_enter = false;
		}
	} catch (boost::filesystem::filesystem_error & e) {
		files[2].push_back(e.code().message());
		can_enter = false;
	}
}

void insert_files(const int & id, const std::string & path) {
	for (boost::filesystem::directory_entry & i : boost::filesystem::directory_iterator(path)) {
		if (!(i.path().filename().string()[0] == '.' && !show_hidden)) {
			files[id].push_back(i.path().filename().string());
		}
	}

	std::sort(files[id].begin(), files[id].end());
}

void run_command() {
	std::string command = cmd_arg.cmd;
	curs_set(1);


	wclear(shell_panel);
	wprintw(shell_panel, (":" + command).c_str());
	wrefresh(shell_panel);
	wmove(shell_panel, 0, command.size() + 1);

	int ch;
	if (command.back() == '\n') {
		command.pop_back();
		ch = '\n';
	} else {
		int ins_pos = command.size();
		while ((ch = getch()) != 27) {
			if (ch == '\n') {
				break;
			} else if (ch == KEY_LEFT) {
				if (ins_pos)
					--ins_pos;
			} else if (ch == KEY_RIGHT) {
				if (ins_pos < command.size())
					++ins_pos;
			} else if (ch == KEY_BACKSPACE) {
				if (!command.empty()) {
					command.erase(ins_pos - 1);
					--ins_pos;
				}
			} else {
				command.insert(ins_pos, 1, ch);
				++ins_pos;
			}

			wclear(shell_panel);
			wprintw(shell_panel, (":" + command).c_str());
			wmove(shell_panel, 0, ins_pos + 1);
			wrefresh(shell_panel);
		}
	}

	if (ch == '\n') {
		size_t sel = command.find(selection_shortcut);
		if (sel != std::string::npos) {
			command.replace(sel, selection_shortcut.size(), files[1][selection]);
		}
		command = "(cd \"" + current_path.string() + "\" && " + command + ")";
		endwin();
		system(command.c_str());
		initscr();
	}
	curs_set(0);
	cbreak();

}

int main(int argc, char ** argv) {
	if (argc > 2) {
		std::cerr << "sfm [path]\n";
		return -1;
	} else if (argc == 2) {
		current_path = argv[1] + (std::string(argv[1]).back() == '/' ? std::string() : std::string("/"));
		if (!boost::filesystem::exists(current_path)) {
			std::cerr << "Directory " << current_path.string() << " does not exist\n";
			return -1;
		}

		if (!boost::filesystem::is_directory(current_path)) {
			std::cerr << "\"" << current_path.string() << "\" is not a directory\n";
		}
	} else {
		current_path = boost::filesystem::current_path();
	}

	start_session();
	mainloop();
	end_session();
	return 0;
}
