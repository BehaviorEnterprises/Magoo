/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#ifndef __SCOPE_H__
#define __SCOPE_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <signal.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
//#include <ft2build.h>
//#include FT_FREETYPE_H
#include <cairo.h>
#include <cairo-xlib.h>
//#include <cairo-ft.h>

typedef struct Col { unsigned char r, g, b, a; } Col;

typedef struct Img Img;
struct Img {
	Img *next;
	Window win;
	cairo_t *ctx;
	int x, y, w, h;
	float scale;
	struct {
		cairo_surface_t *pix;
		char *name;
		unsigned char alpha;
	} source;
	struct {
		cairo_surface_t *pix;
		Col low, hi, pseudo;
	} threshold;
	struct {
		cairo_surface_t *pix;
		int x, y, w, h;
		Col line;
	} crop;
};

typedef struct Conf {
	struct { Col low, hi, pseudo; } threshold;
	Col line;
	unsigned char alpha;
	const char *prompt;
} Conf;

typedef struct Command {
	const char *name;
	int (*func)(const char *);
	const char *help;
	const char *detail;
} Command;

/* cairo.c */
Img *cairo_create_img(const char *);
int cairo_destroy_img(Img *);
int cairo_free_img(Img *);
int cairo_init_img(Img *);
int img_draw(Img *);

/* commands.c */
int command_init();
int command(const char *);

/* console.c */
int console_init(int, const char **);

/* config.c */
int config_init();
int config_free();

/* scope.c */
void die(const char *, ...);

/* xlib.c */
int main_loop();
int xlib_init();
int xlib_free();

/* global data */
Display *dpy;
int scr, dep;
Visual *vis;
GC gc;
int ww, wh, input, terminal;
Bool running;
Img *imgs;
Img *focused_img;
Conf conf;
Command *commands;

#endif /* __SCOPE_H__ */

