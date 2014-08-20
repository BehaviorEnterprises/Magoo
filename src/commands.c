/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

#define GET_FOCUSED_IMG\
	if (!focused_img) {\
		fprintf(stderr, "threshold: no image focused\n");\
		return 1;\
	}

static int cmd_alpha(const char *);
static int cmd_area(const char *);
static int cmd_clear(const char *);
static int cmd_color(const char *);
static int cmd_count(const char *);
static int cmd_echo(const char *);
static int cmd_help(const char *);
static int cmd_list(const char *);
static int cmd_name(const char *);
static int cmd_quit(const char *);
static int cmd_ratio(const char *);
static int cmd_shell(const char *);
static int cmd_sink(const char *);
static int cmd_threshold(const char *);

static unsigned char *data_ptr, *dptr;
static int data_stride;
static Col *col_low, *col_hi;
static Bool check_crop_area;
static FILE *out = NULL;
Command _commands[] = {
	{ "alpha",     cmd_alpha,     "set/show the alpha level for the original image",
		NULL },
	{ "area",      cmd_area,      "calculate the area of the current crop region",
		NULL },
	{ "clear",     cmd_clear,     "clear the current crop boundaries",
		NULL },
	{ "color",     cmd_color,     "set/show the current highlight color",
		NULL },
	{ "count",     cmd_count,     "calculate the area of the current crop region between the thresholds",
		NULL },
	{ "echo",      cmd_echo,      "write a string to the output",
		NULL },
	{ "exit",      cmd_quit,      "exit",
		NULL },
	{ "help",      cmd_help,      "show this help menu",
		NULL },
	{ "list",      cmd_list,      "list all open images",
		NULL },
	{ "name",      cmd_name,      "name of the focused image",
		NULL },
	{ "quit",      cmd_quit,      NULL,
		NULL },
	{ "ratio",     cmd_ratio,     "calculate the ratio of the current crop region",
		NULL },
	{ "shell",     cmd_shell,     "display the output of a shell command",
		NULL },
	{ "sink",      cmd_sink,      "sink output to file",
		NULL },
	{ "threshold", cmd_threshold, "set/show the current threshold",
		"[spec] val val [val [val val val]]\nDetails coming soon" },
	{ NULL, NULL, NULL, NULL },
};


int command(const char *s) {
	if (!out) out = stdout;
	if (!s || !strlen(s)) return 1;
	if (*s == '#') return 0;
	char *arg = strchr(s,' ');
	while (arg && arg[0] == ' ') arg++;
	if (arg && ! arg[0]) arg = NULL;
	Command *cmd = commands;
	for (cmd = commands; cmd->name; cmd++)
		if (!strncasecmp(s, cmd->name, 3))
			return cmd->func(arg);
	char *base = strdup(s);
	arg = strchr(base,' ');
	if (arg) *arg = '\0';
	printf("scope: %s: command not found\n", base);
	free(base);
	return 1;
}

int command_init() {
	commands = _commands;
	return 0;
}



static int _calc_area(cairo_t *ctx, int i, int j) {
	if (check_crop_area && !cairo_in_clip(ctx, i, j)) return 0;
	else return 1;
}

static int _calc_count(cairo_t *ctx, int i, int j) {
	if (check_crop_area && !cairo_in_clip(ctx, i, j)) return 0;
	dptr = data_ptr + j * data_stride + i;
	if (*dptr == 255) return 1;
	return 0;
}

static int _print_area(long area) {
	fprintf(out,"AREA: %ld\n", area);
	return 0;
}

static int _print_count(long area) {
	fprintf(out,"COUNT: %ld\n", area);
	return 0;
}

static int _print_null(long area) {
	return 0;
}

static long _calculate(int (*calc)(cairo_t *, int, int), int (*print)(long)) {
	GET_FOCUSED_IMG
	Img *img = focused_img;
	int i, j;
	double x1, x2, y1, y2;
	long area = 0;
	cairo_clip_preserve(img->ctx);
	cairo_clip_extents(img->ctx, &x1, &y1, &x2, &y2);
	cairo_surface_flush(img->threshold.pix);
	data_stride = cairo_image_surface_get_stride(img->threshold.pix);
	data_ptr = cairo_image_surface_get_data(img->threshold.pix);
	col_low = &img->threshold.low;
	col_hi = &img->threshold.hi;
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0)
		check_crop_area = False;
	else
		check_crop_area = True;
	for (i = 0; i < img->w; i++)
		for (j = 0; j < img->h; j++)
			area += calc(img->ctx, i, j);
	cairo_reset_clip(img->ctx);
	print(area);
	return area;
}



int cmd_alpha(const char *arg) {
	GET_FOCUSED_IMG
	if (!arg) fprintf(out, "A-%03d\n", focused_img->source.alpha);
	else sscanf(arg, "%hhu", &focused_img->source.alpha);
	return 0;
}

int cmd_area(const char *arg) {
	_calculate(_calc_area, _print_area);
}

int cmd_clear(const char *arg) {
	GET_FOCUSED_IMG
	cairo_new_path(focused_img->ctx);
	img_draw(focused_img);
	return 0;
}

int cmd_count(const char *arg) {
	_calculate(_calc_count, _print_count);
}

int cmd_color(const char *arg) {
	GET_FOCUSED_IMG
	Col *c = &focused_img->threshold.pseudo;
	if (!arg) {
		fprintf(out, "RGBA: %03d %03d %03d %03d\n", c->r, c->g, c->b, c->a);
		return 0;
	}
	if (arg[0] < 48 || arg[0] > 57) c->a = 0;
	else {
		int ret = sscanf(arg, "%hhu %hhu %hhu %hhu", &c->r, &c->g, &c->b, &c->a);
		if (ret < 4) c->a = 255;
	}
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

int cmd_echo(const char *arg) {
	if (arg) fprintf(out, "%s\n", arg);
	else fprintf(out, "\n");
	return 0;
}

int cmd_help(const char *arg) {
	Command *cmd = commands;
	// TODO
	if (!arg) fprintf(out,"\n\033[1mCOMMANDS\033[0m\n");
	for (cmd = commands; cmd->name; cmd++) {
		if (arg && !strncasecmp(arg, cmd->name, 3)) {
			if (cmd->detail) fprintf(out, "\n\033[1m%s\033[0m    %s\n",
					cmd->name, cmd->detail);
			else fprintf(out, "\n\033[1m%s\033[0m\n%s\n",
					cmd->name, cmd->help);
			return 0;
		}
		else if (!arg && cmd->help) {
			fprintf(out, "  \033[1m%s\033[0m\n    %s\n", cmd->name, cmd->help);
		}
	}
	// TODO
}

int cmd_list(const char *arg) {
	Img *img;
	int i = 0;
	for (img = imgs; img; img = img->next) {
		i++;
		if (focused_img == img)
			fprintf(out, "(%d) %s\n", i, img->source.name);
		else
			fprintf(out, " %d  %s\n", i, img->source.name);
	}
}

int cmd_name(const char *arg) {
	GET_FOCUSED_IMG
	fprintf(out,"NAME: %s\n", focused_img->source.name);
	return 0;
}

int cmd_ratio(const char *arg) {
	long count, area;
	count = _calculate(_calc_count, _print_null);
	area = _calculate(_calc_area, _print_null);
	fprintf(out, "RATIO: %Lf\n", count / (long double) area);
	return 0;
}

int cmd_quit(const char *arg) {
	running = False;
	return 0;
}

int cmd_shell(const char *arg) {
	FILE *in = popen(arg, "r");
	char line[256];
	while (fgets(line, 256, in))
		fprintf(out, line);
	pclose(in);
	return 0;
}

int cmd_sink(const char *arg) {
	if (!arg && out != stdout) {
		fclose(out);
		out = stdout;
		return 0;
	}
	FILE *new = fopen(arg, "a");
	if (!new) {
		perror("[SCOPE]");
		out = stdout;
		return 0;
	}
	out = new;
}

int cmd_threshold(const char *arg) {
	GET_FOCUSED_IMG
	Col *low = &focused_img->threshold.low;
	Col *hi = &focused_img->threshold.hi;
	if (!arg) {
		fprintf(out, "     R   G   B \n");
		fprintf(out, "LO: %03d %03d %03d\nHI: %03d %03d %03d\n",
				low->r, low->g, low->b, hi->r, hi->g, hi->b);
		return;
	}
	switch (arg[0]) {
		case 'r': case 'R':
			sscanf(arg, "%*s %hhu %hhu", &low->r, &hi->r); break;
		case 'g': case 'G':
			sscanf(arg, "%*s %hhu %hhu", &low->g, &hi->g); break;
		case 'b': case 'B':
			sscanf(arg, "%*s %hhu %hhu", &low->b, &hi->b); break;
		case 'l': case 'L':
			sscanf(arg, "%*s %hhu %hhu %hhu", &low->r, &low->g, &low->b); break;
		case 'h': case 'H':
			sscanf(arg, "%*s %hhu %hhu %hhu", &hi->r, &hi->g, &hi->b); break;
		default:
			sscanf(arg, "%hhu %hhu %hhu %hhu %hhu %hhu",
					&low->r, &low->g, &low->b, &hi->r, &hi->g, &hi->b); break;
	}
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}


