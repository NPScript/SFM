
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


std::map<std::string, char> NAV_KEYS = {
	{"up", 'k'},
	{"down", 'j'},
	{"left", 'h'},
	{"right", 'l'},
	{"toggle_hidden", 'H'},
	{"open", 'o'},
	{"quit", 'q'},
	{"execute", '!'}
};

/* 
 * This are shortcuts, 
 * which just executes a shell command
 * and displays its output if existing.
 *
 * A '\n' means it directly executes this command
 * if no '\n' is at the end its possible to edit the comand
 *
 * A '\?' means it replaces this sign with the current selection in the manager
 * */

std::map<char, std::string> SHORT_CUTS = {
	{'m', "mkdir "},
	{'d', "rm "},
	{' ', "less -N +k \?\n"}
};

