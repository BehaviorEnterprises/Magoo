/**********************************************************************\
* MAGOO - Image analysis software
*
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
*
*    This program is free software: you can redistribute it and/or
*    modify it under the terms of the GNU General Public License as
*    published by the Free Software Foundation, either version 3 of the
*    License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see
*    <http://www.gnu.org/licenses/>.
*
\**********************************************************************/

#include "magoo.h"

static FILE *log_file;
static Bool ready_for_input = False;

void die(const char *fmt, ...) {
	fprintf(stderr,"[SCOPE] ");
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr,"\n");
	exit(1);
}

int main(int argc, const char **argv) {
	focused_img = NULL;
	imgs = NULL;
	command_init();
	console_init(argc, argv);
	log_file = stderr;
	imgs = NULL;
	xlib_init();
	config_init(argc, argv);
	main_loop();
	// close all files
	xlib_free();
	config_free();
	return 0;
}
