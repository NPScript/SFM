
/* Available colors:
 * COLOR_BLACK   
 * COLOR_RED     
 * COLOR_GREEN   
 * COLOR_YELLOW  
 * COLOR_BLUE    
 * COLOR_MAGENTA 
 * COLOR_CYAN    
 * COLOR_WHITE   
*/


std::vector<std::pair<int, int>> ITEM_COLORS = {
 {7, 0}, 						/* default */
 {COLOR_GREEN, 0},	/* border */
 {0, COLOR_GREEN}		/* highlight color */
};


std::map<char, std::pair<void(*)(void), arg>> NAV_KEYS = {
	{'k', {&move, {.h = -1}}},
	{'j', {&move, {.h =  1}}},
	{'h', {&move, {.v = -1}}},
	{'l', {&move, {.v =	1}}},
	{'g', {&move, {.cmd =	"top"}}},
	{'G', {&move, {.cmd =	"bottom"}}},
	{'H', {&toggle_hidden, {}}},
	{'o', {&open, {}}},
	{'q', {&quit, {}}},
	{'!', {&run_command, {}}}
};

std::string selection_shortcut = "<sel>";

/* 
 * This are shortcuts, 
 * which just executes a shell command
 * and displays its output if existing.
 *
 * A '\n' means it directly executes this command
 * if no '\n' is at the end its possible to edit the comand
 *
 * The selection_shortcut will be replaced with the current selection in the manager
 * */

std::map<char, std::string> SHORT_CUTS = {
	{'m', "mkdir "},
	{'d', "rm "},
	{' ', "less -N +k " + selection_shortcut + "\n"},
	{'s', getenv("SHELL") + std::string("\n")}
};

std::map<std::string, std::string> DEF_PROGS = {
	{".pdf", 		"zathura"},
	{".png", 		"sxiv"},
	{".jpg", 		"sxiv"},
	{".jpeg", 	"sxiv"},
	{".jpeg", 	"sxiv"},
	{".mp4", 		"mpv"},
	{"fallback", "vim"}
};
