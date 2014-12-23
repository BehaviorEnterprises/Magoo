
{ "alpha",     cmd_alpha,     "set/show the alpha level for the original image",
	"[val]\n\n"
	"* Set or show the alpha (opacity) level of the background image\n"
	"* With no paramaters, \033[4malpha\033[0m returns the current setting\n"
	"* A single numerical \033[4mval\033[0m from 0-255 can be specified to set the level" },

{ "area",      cmd_area,      "alias for \033[4mcount total\033[0m",
	NULL },

{ "clear",     cmd_clear,     "clear the current crop boundaries",
	NULL },

{ "close",     cmd_close,     "close the current image",
	NULL },

{ "color",     cmd_color,     "set/show the current highlight color",
	"[val AARRGGBB]\n\n"
	"* Set or show the color used to highlight points within the threshold criteria\n"
	"* With no paramaters, \033[4mcolor\033[0m returns the current settings\n"
	"* To set a color, the first parameter should specify which range color to adjust\n"
	"* Color is specified as an alpha, red, green, blue, hex code (00-FF for each)\n"
	"* Setting alpha is optional - with none specified, fully opaque (FF) will be assumed\n"
	"* If the alpha level is set to 00, the original image will be used at full opacity" },

{ "count",     cmd_count,     "calculate the area of the currently highlighted regions",
	"[val | total]\n\n"
	"* If \033[4mval\033[0m is specified, the area of that highlight region is displayed\n"
	"* If \033[4mtotal\033[0m is specified, the area of the current crop region is displayed\n"
	"* With no parameter, values for each region are displayed" },

{ "data",      cmd_data,      "display the current data file contents",
	NULL },

{ "exit",      cmd_quit,      "exit",
	NULL },

{ "help",      cmd_help,      "show this help menu or learn about commands",
	"[command]\n\n"
	"* If \033[4mcommand\033[0m is specified, detailed help will be shown for that command\n"
	"* Without a \033[4mcommand\033[0m parameter, show the main help menu" },

{ "info",      cmd_info,      "get color info for the selected region",
	NULL }, //TODO?

{ "layer",      cmd_layer,      "toggle highlight layer visibility",
	NULL },

{ "list",      cmd_list,      "list all open images",
	NULL },

{ "mouse",     cmd_mouse,      "set/show the mouse mode",
	"[polygon | draw RRGGBB]\n\n"
	"* Specify \033[4mpolygon\033[0m to select crop regions with the mouse\n"
	"* Speficy \033[4mdraw\033[0m followed by a hex color code to modify the image\n"
	"* Speficy \033[4mnote\033[0m to add note marks to the image\n"
	"* In the absence of a parameter, the current setting is displayed" },

{ "move",      cmd_move,      "move image window",
	"[x y]\n\n"
	"* Set or show the selected image window position\n"
	"* With no paramaters, \033[4mmove\033[0m returns the current coordinates\n"
	"* Two integer values can be specified for the \033[4mx\033[0m and \033[4my\033[0m coordinates\n"
	"* Anything other than a numeric parameter will move the window to 0 0\n"
	"* Moving may be required after a zoom, as the window may be offscreen" },

{ "name",      cmd_name,      "name of the focused image",
	"[string]\n\n"
	"* Set or show the selected image window name\n"
	"* With no paramaters, \033[4mname\033[0m returns the current name\n"
	"* If \033[4mstring\033[0m is provided, the current image name will be set\n"
	"* Setting the image name currently serves no purpose\n"
	"* The default name is the basename of the file without the extension" },

{ "note",      cmd_note,      "write a string to the output",
	"[string]\n\n"
	"* Writes \033[4mstring\033[0m to the current output\n"
	"* Can be used to place notes in output sink files or data files" },

{ "open",      cmd_open,      "open a new image",
	"[filename]\n\n"
	"* Open \033[4mfilename\033[0m as a new image buffer"},

{ "polygon",   cmd_poly,      "specify a selection polygon geometry",
	"[x1,y1 x2,y2 x3,y3 ...]\n\n"
	"* Select exact image coordinates for a crop region\n"
	"* Usefule for selecting a consistent area across several images\n"
	"* Specify three or more coordinates as \033[4mx,y\033[0m separated by spaces" },

{ "quit",      cmd_quit,      "quit",
	NULL },

{ "ratio",     cmd_ratio,     "calculate the ratio(s) of the current crop region",
	"[relative | val | val val]\n\n"
	"* Calculate the ratio of \033[4mcount\033[0m to \033[4marea\033[0m for a region\n"
	"* Specify region \033[4mval\033[0m select a specific region\n"
	"* Specify two \033[4val\033[0ms for the ratio of the first over the second\n"
	"* Specify \033[4mrelative\033[0m for the ratio of each region over the total of all regions\n"
	"* Ratios are not scaled to percents and will range from 0.00 - 1.00\n" },

{ "shell",     cmd_shell,     "display the output of a shell command",
	"command-string\n\n"
	"* Executes \033[4mcommand-string\033[0m in the system shell and returns the output\n"
	"* CAUTION: any commands that expect input from stdin may have unpredictable results" },

{ "sink",      cmd_sink,      "sink output to file",
	"[filename]\n\n"
	"* Redirects all subsequent output to \033[4mfilename\033[0m\n"
	"* Output is appended, \033[4mfilename\033[0m will never be overwritten\n"
	"* In the absence of a parameter, \033[4msink\033[0m will end any previous redirection" },

{ "stretch",   cmd_stretch,   "stretch - documentation needed",
	NULL },

{ "threshold", cmd_threshold, "set/show the current threshold",
	"[val RRGGBB RRGGBB]\n\n"
	"* Set or show the threshold level(s) for the current image\n"
	"* With no parameter, \033[4mthreshold\033[0m returns the current settings\n"
	"* Specify region \033[4mval\033[0m to set the range for that region as two RRGGBB hex color codes" },

{ "zoom",     cmd_zoom,       "zoom/scale image",
	"[direction | val]\n\n"
	"* Set or show the zoom or scaling level for the current image\n"
	"* With no parameter, \033[4mzoom\033[0m returns the current scaling\n"
	"* One parameter can be provided to specify a direction or value\n"
	"* Directions can be specified as \033[4mup\033[0m, \033[4mdown\033[0m, \033[4min\033[0m, \033[4mout\033[0m\n"
	"* Values can range from 0.10 to 1.00 and speficy an exact scale" },

{ NULL, NULL, NULL, NULL },
