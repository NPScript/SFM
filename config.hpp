
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

std::map<std::string, std::pair<std::string, int>> DEF_PROGS = {
	{".pdf", 		{"zathura", 1}},
	{".png", 		{"sxiv", 1}},
	{".jpg", 		{"sxiv", 1}},
	{".jpeg", 	{"sxiv", 1}},
	{".jpeg", 	{"sxiv", 1}},
	{".mp4", 		{"mpv", 1}},
	{".mp3", 		{"mpv", 0}},
	{"fallback", {"nvim", 0}}
};

std::string DIRECTORY_ICON = "";

std::map<std::string, std::string> ICONS_SUFFIX = {
	{"", "?"},
	{".7z", ""},
	{".txt", "🖹"},
	{".a", ""},
	{".ai", ""},
	{".apk", ""},
	{".asm", ""},
	{".asp", ""},
	{".aup", ""},
	{".avi", ""},
	{".awk", ""},
	{".bash", ""},
	{".bat", ""},
	{".bmp", ""},
	{".svg", ""},
	{".bz2", ""},
	{".c", ""},
	{".c++", ""},
	{".cab", ""},
	{".cbr", ""},
	{".cbz", ""},
	{".cc", ""},
	{".class", ""},
	{".clj", ""},
	{".cljc", ""},
	{".cljs", ""},
	{".cmake", ""},
	{".coffee", ""},
	{".conf", ""},
	{".cp", ""},
	{".cpio", ""},
	{".cpp", ""},
	{".cs", ""},
	{".csh", ""},
	{".css", ""},
	{".cue", ""},
	{".cvs", ""},
	{".cxx", ""},
	{".d", ""},
	{".dart", ""},
	{".db", ""},
	{".deb", ""},
	{".diff", ""},
	{".dll", ""},
	{".doc", ""},
	{".docx", ""},
	{".dump", ""},
	{".edn", ""},
	{".eex", ""},
	{".efi", ""},
	{".ejs", ""},
	{".elf", ""},
	{".elm", ""},
	{".epub", ""},
	{".erl", ""},
	{".ex", ""},
	{".exe", ""},
	{".exs", ""},
	{".f#", ""},
	{".fifo", "|"},
	{".fish", ""},
	{".flac", ""},
	{".flv", ""},
	{".fs", ""},
	{".fsi", ""},
	{".fsscript", ""},
	{".fsx", ""},
	{".gem", ""},
	{".gemspec", ""},
	{".gif", ""},
	{".go", ""},
	{".gz", ""},
	{".gzip", ""},
	{".h", ""},
	{".haml", ""},
	{".hbs", ""},
	{".hh", ""},
	{".hpp", ""},
	{".hrl", ""},
	{".hs", ""},
	{".htaccess", ""},
	{".htm", ""},
	{".html", ""},
	{".htpasswd", ""},
	{".hxx", ""},
	{".ico", ""},
	{".img", ""},
	{".ini", ""},
	{".iso", ""},
	{".jar", ""},
	{".java", ""},
	{".jl", ""},
	{".jpeg", ""},
	{".jpg", ""},
	{".js", ""},
	{".json", ""},
	{".jsx", ""},
	{".key", ""},
	{".ksh", ""},
	{".leex", ""},
	{".less", ""},
	{".lha", ""},
	{".lhs", ""},
	{".log", ""},
	{".lua", ""},
	{".lzh", ""},
	{".lzma", ""},
	{".m4a", ""},
	{".m4v", ""},
	{".markdown", ""},
	{".md", ""},
	{".mdx", ""},
	{".mjs", ""},
	{".mkv", ""},
	{".ml", "λ"},
	{".mli", "λ"},
	{".mov", ""},
	{".mp3", ""},
	{".mp4", ""},
	{".mpeg", ""},
	{".mpg", ""},
	{".msi", ""},
	{".mustache", ""},
	{".nix", ""},
	{".o", ""},
	{".ogg", ""},
	{".pdf", ""},
	{".php", ""},
	{".pl", ""},
	{".pm", ""},
	{".png", ""},
	{".pp", ""},
	{".ppt", ""},
	{".pptx", ""},
	{".ps1", ""},
	{".psb", ""},
	{".psd", ""},
	{".pub", ""},
	{".py", ""},
	{".pyc", ""},
	{".pyd", ""},
	{".pyo", ""},
	{".r", "ﳒ"},
	{".rake", ""},
	{".rar", ""},
	{".rb", ""},
	{".rc", ""},
	{".rlib", ""},
	{".rmd", ""},
	{".rom", ""},
	{".rpm", ""},
	{".rproj", "鉶"},
	{".rs", ""},
	{".rss", ""},
	{".rtf", ""},
	{".s", ""},
	{".sass", ""},
	{".scala", ""},
	{".scss", ""},
	{".sh", ""},
	{".slim", ""},
	{".sln", ""},
	{".so", ""},
	{".sql", ""},
	{".styl", ""},
	{".suo", ""},
	{".swift", ""},
	{".t", ""},
	{".tar", ""},
	{".tex", "ﭨ"},
	{".tgz", ""},
	{".toml", ""},
	{".ts", ""},
	{".tsx", ""},
	{".twig", ""},
	{".vim", ""},
	{".vimrc", ""},
	{".vue", "﵂"},
	{".wav", ""},
	{".webm", ""},
	{".webmanifest", ""},
	{".webp", ""},
	{".xbps", ""},
	{".xcplayground", ""},
	{".xhtml", ""},
	{".xls", ""},
	{".xlsx", ""},
	{".xml", ""},
	{".xul", ""},
	{".xz", ""},
	{".yaml", ""},
	{".yml", ""},
	{".zip", ""},
	{".zsh", ""},
};
