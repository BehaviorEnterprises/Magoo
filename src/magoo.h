/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#ifndef __MAGOO_H__
#define __MAGOO_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <readline/readline.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
//#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include <cairo-xlib.h>

#define STRING(s)		STRINGIFY(s)
#define STRINGIFY(s)	#s

#define VERSION_STRING	\
STRING(PROGRAM_NAME) " v" STRING(PROGRAM_VER) "\n" \
"Copyright (C) 2014 Behavior Enterprises <http://behaviorenterprises.com>\n" \
"License GPL3: GNU GPL version 3 <http://gnu.org/licenses/gpl.html>\n" \
"Written by Jesse McClure\n"

enum { MODE_DRAW = 0, MODE_POLY };

typedef union {
	uint32_t u;
	struct { uint8_t b, g, r, a; };
} Col;

typedef struct Threshold {
	Col low, hi, pseudo;
} Threshold;

typedef struct Img Img;
struct Img {
	Img *next;
	Window win;
	cairo_t *ctx;
	int x, y, w, h;
	float scale;
	cairo_surface_t *source;
	char *name;
	uint8_t alpha;
	cairo_surface_t **mask;
//	Col cropline;
};

typedef struct Conf {
	Threshold *thresh;
	Col line, draw;
	uint8_t alpha, levels;
	char *prompt;
	Bool layers;
} Conf;

typedef struct Command {
	const char *name;
	int (*func)(const char *);
	const char *help;
	const char *detail;
} Command;

/* cairo.c */
Img *cairo_image_init(const char *);
int cairo_image_free();
int img_draw(Img *);

/* commands.c */
int command_init();
int command(const char *);

/* console.c */
int console_init(int, const char **);

/* config.c */
int config_init();
int config_free();

/* magoo.c */
void die(const char *, ...);

/* xlib.c */
int main_loop();
int xlib_dock(int);
int xlib_init();
int xlib_free();

/* global data */
Display *dpy;
int scr, dep;
Visual *vis;
GC gc;
int ww, wh, input, terminal;
Bool running;
Img *imgs, *focused_img;
Conf conf;
Command *commands;

#endif /* __MAGOO_H__ */

