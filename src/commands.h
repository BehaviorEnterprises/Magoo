
{ "alpha",     cmd_alpha,     "set/show the alpha level for the original image",
	"[val]\n\n"
	"* Set or show the alpha (opacity) level of the background image\n"
	"* With no paramaters, \033[3malpha\033[0m returns the current setting\n"
	"* A single numerical value from 0-255 can be specified to set the level\n"
	"* Default = 64" },

{ "area",      cmd_area,      "calculate the area of the current crop region",
	NULL },

{ "clear",     cmd_clear,     "clear the current crop boundaries",
	NULL },

{ "close",     cmd_close,     "close the current image",
	NULL },

{ "color",     cmd_color,     "set/show the current highlight color",
	"[val val val [val]]\n\n"
	"* Set or show the color used to highlight points within the threshold criteria\n"
	"* With no paramaters, \033[3mcolor\033[0m returns the current setting\n"
	"* Four numerical values from 0-255 can be specified to set the Red, Green, Blue, and Alpha levels\n"
	"* In the absence of a fourth parameter, an alpha value of 255 (fully opaque) will be used\n"
	"* If the alpha level is set to 0, the original image will be used at full opacity" },

{ "count",     cmd_count,     "calculate the area of the current crop region within the threshold criteria",
	NULL },

{ "echo",      cmd_echo,      "write a string to the output",
	"[string]\n\n"
	"* Writes \033[3mstring\033[0m to the current output\n"
	"* Can be used to place notes in output sink files" },

{ "exit",      cmd_quit,      "exit",
	NULL },

{ "help",      cmd_help,      "show this help menu or learn about commands",
	"[command]\n\n"
	"* If \033[3mcommand\033[0m is specified, detailed help will be shown for that command\n"
	"* Without a \033[3mcommand\033[0m parameter, show the main help menu" },

{ "list",      cmd_list,      "list all open images",
	NULL },

{ "move",      cmd_move,      "move image window",
	"[x y]\n\n"
	"* Set or show the selected image window position\n"
	"* With no paramaters, \033[3mmove\033[0m returns the current coordinates\n"
	"* Two integer values can be specified for the x and y coordinates\n"
	"* Anything other than a numeric parameter will move the window to 0 0\n"
	"* Moving may be required after a zoom, as the window may be offscreen" },

{ "name",      cmd_name,      "name of the focused image",
	"[string]\n\n"
	"* Set or show the selected image window name\n"
	"* With no paramaters, \033[3mname\033[0m returns the current name\n"
	"* If \033[3mstring\033[0m is provided, the current image name will be set\n"
	"* Setting the image name currently serves no purpose\n"
	"* The default name is the basename of the file without the extension" },

{ "open",      cmd_open,      "open a new image",
	"[filename]\n\n"
	"* Open \033[3mfilename\033[0m as a new image buffer"},

{ "quit",      cmd_quit,      NULL,
	NULL },

{ "ratio",     cmd_ratio,     "calculate the ratio of the current crop region",
	"\n\n"
	"* Calculate the ratio of \033[3mcount\033[0m to \033[3marea\033[0m for the current crop region\n"
	"* Ratios are not scaled to percents and will range from 0.0 - 1.0" },

{ "shell",     cmd_shell,     "display the output of a shell command",
	"command-string\n\n"
	"* Executes \033[3mcommand-string\033[0m in the system shell and returns the output\n"
	"* CAUTION: any commands that expect input from stdin may have unpredictable results" },

{ "sink",      cmd_sink,      "sink output to file",
	"[filename]\n\n"
	"* Redirects all subsequent output to \033[3mfilename\033[0m\n"
	"* Output is appended, \033[3mfilename\033[0m will never be overwritten\n"
	"* In the absence of a parameter, \033[3msink\033[0m will end any previous redirection" },

{ "threshold", cmd_threshold, "set/show the current threshold",
	"[spec] val val [val [val val val]]\n\n"
	"* Set or show the threshold level(s) for the current image\n"
	"* With no parameter, \033[3mthreshold\033[0m returns the current setting\n"
	"* Optional parameter \033[3mspec\033[0m can specify on of the following:\n"
	"    - Color:    Red, Green, Blue\n"
	"    - Boundary: Hi, Low\n"
	"* Provided with a color \033[3mspec\033[0m, threshold will read 2 values: hi and low for the specified color\n"
	"* Provided with a boundary \033[3mspec\033[0m, threshold will read 3 values: red, green, and blue values for that boundary\n"
	"* With no \033[mspec\033[0m, threshold requires 6 values: red, green, and blue for low then hi boundaries\n"
	"* All \033[mval\033[0m's can range from 0-255\n"
	"* Default = 0 0 0 80 80 80" },

{ "zoom",     cmd_zoom,       "zoom/scale image",
	"[direction | val]\n\n"
	"* Set or show the zoom or scaling level for the current image\n"
	"* With no parameter, \033[3mzoom\033[0m returns the current scaling\n"
	"* One parameter can be provided to specify a direction or value\n"
	"* Directions can be specified as \033[3mup\033[0m, \033[3mdown\033[0m, \033[3min\033[0m, \033[3mout\033[0m\n"
	"* Values can range from 0.1 to 1.0 and speficy an exact scale" },

{ NULL, NULL, NULL, NULL },
