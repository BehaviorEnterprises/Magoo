/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

const char *cmd_names[MAX_COLUMNS] = { "area", "count", "ratio", "note", NULL };

#define GET_FOCUSED_IMG\
	if (!focused_img) {\
		fprintf(stderr, "no image focused\n");\
		return 1;\
	}

static int cmd_alpha(const char *);
static int cmd_area(const char *);
static int cmd_clear(const char *);
static int cmd_close(const char *);
static int cmd_color(const char *);
static int cmd_count(const char *);
static int cmd_data(const char *);
static int cmd_help(const char *);
static int cmd_info(const char *);
static int cmd_layer(const char *);
static int cmd_list(const char *);
static int cmd_mouse(const char *);
static int cmd_move(const char *);
static int cmd_name(const char *);
static int cmd_note(const char *);
static int cmd_open(const char *);
static int cmd_poly(const char *);
static int cmd_quit(const char *);
static int cmd_ratio(const char *);
static int cmd_shell(const char *);
static int cmd_sink(const char *);
static int cmd_stretch(const char *);
static int cmd_threshold(const char *);
static int cmd_zoom(const char *);

static FILE *out = NULL;
static Command _commands[] = {
#include "commands.h"
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
		if (!strncasecmp(s, cmd->name, strlen(cmd->name)))
			return cmd->func(arg);
	char *base = strdup(s);
	arg = strchr(base,' ');
	if (arg) *arg = '\0';
	printf("magoo: %s: command not found\n", base);
	free(base);
	return 1;
}

int command_init() {
	commands = _commands;
	return 0;
}

int image_load(const char *fname) {
	char *clean, *home = getenv("HOME");
	if (strncmp(fname, "file://", 7) == 0) clean = strdup(fname + 7);
	else if (strncmp(fname, "~/", 2) == 0 && home) {
		clean = malloc(strlen(fname) + strlen(home));
		strcpy(clean, home);
		strcat(clean, fname + 1);
	}
	else clean = strdup(fname);
	char *end = clean + strlen(clean) - 1;
	while (end > clean && isspace(*end)) end--;
	*(end+1) = '\0';
	Img *img = cairo_image_init(clean);
	free(clean);
	if (!img) return 1;
	img->next = imgs;
	imgs = img;
	focused_img = img;
	XMapWindow(dpy, focused_img->win);
	XFlush(dpy);
	return 0;
}

int image_unload(Img *img) {
	Img *pre;
	for (pre = imgs; pre && pre->next != img; pre = pre->next);
	if (pre) pre->next = img->next;
	else imgs = img->next;
	focused_img = img->next;
	if (!focused_img) focused_img = imgs;
	cairo_image_free(img);
	return 0;
}


/****************************\
|* Local "helper" functions
\****************************/

static long *_calculate(long *total) {
	long *ret = (long *) calloc(conf.levels, sizeof(long));
	if (!focused_img) {
		fprintf(stderr, "no image focused\n");
		return ret;
	}
	Img *img = focused_img;
	int i, j, n;
	double x1, x2, y1, y2;
	long area = 0;
	cairo_clip_preserve(img->ctx);
	cairo_clip_extents(img->ctx, &x1, &y1, &x2, &y2);
	uint8_t full, *dptr;
	int dstride;
	Bool crop = True;
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) crop = False;
	for (n = 0; n < conf.levels; ++n) {
		cairo_surface_flush(img->mask[n]);
		dptr = cairo_image_surface_get_data(img->mask[n]);
		dstride = cairo_image_surface_get_stride(img->mask[n]);
		ret[n] = 0;
		full = conf.thresh[n].pseudo.a;
		if (full == 0) full = 255;
		for (i = 0; i < img->w; i++) {
			for (j = 0; j < img->h; j++) {
				if (crop && !cairo_in_clip(img->ctx, i, j)) continue;
				//ret[n] += ( *(dptr + j * dstride + i) == 255 ? 1 : 0);
				ret[n] += ( *(dptr + j * dstride + i) == full ? 1 : 0);
				if (n == 0) area += 1;
			}
		}
	}
	cairo_reset_clip(img->ctx);
	*total = area;
	return ret;
}

int dat_printf(const char *name, const char *fmt, ...) {
	va_list arg;
	/* print to output or sink */
	va_start(arg, fmt);
	vfprintf(out, fmt, arg);
	va_end(arg);
	fprintf(out, "\n");
	/* determine notes column */
	int i, n = 0;
	for (i = 0; focused_img->columns[i]; ++i)
		if (strncasecmp(name, cmd_names[focused_img->columns[i] - 1], strlen(name)) == 0)
			break;
	if (!focused_img->columns[i]) return 0;
	/* print to temp string */
	char str[256];
	va_start(arg, fmt);
	vsnprintf(str, 255, fmt, arg);
	va_end(arg);
	/* set string to notes column */
	note_entry(focused_img, i, str);
}


/****************************\
|* Command functions
\****************************/

int cmd_alpha(const char *arg) {
	GET_FOCUSED_IMG
	if (!arg) dat_printf("alpha", "%03d", focused_img->alpha);
	else {
		sscanf(arg, "%hhu", &focused_img->alpha);
		img_draw(focused_img);
		XFlush(dpy);
	}
	return 0;
}

int cmd_area(const char *arg) {
	command("count total");
}

int cmd_clear(const char *arg) {
	GET_FOCUSED_IMG
	cairo_new_path(focused_img->ctx);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

int cmd_close(const char *arg) {
	GET_FOCUSED_IMG
	image_unload(focused_img);
	return 0;
}

int cmd_count(const char *arg) {
	long total;
	long *ret = _calculate(&total);
	uint8_t n;
	if (!arg) {
		for (n = 0; n < conf.levels; ++n)
			fprintf(out, "%d: %ld\n", n + 1, ret[n]);
		fprintf(out, "Total: ");
		dat_printf("count", "%ld", total);
	}
	else if (arg[0] == 't' || arg[0] == 'a')
		dat_printf("count", "%ld", total);
	else if ( (n=atoi(arg)) && n && n <= conf.levels )
		dat_printf("count", "%ld", ret[n-1]);
	else command("help count");
	free(ret);
	return 0;
}

int cmd_color(const char *arg) {
	GET_FOCUSED_IMG
	Col *c;
	uint32_t u;
	uint8_t n;
	if (!arg) {
		fprintf(out, "   AARRGGBB\n");
		for (n = 0; n < conf.levels; ++n) {
			c = &conf.thresh[n].pseudo;
			fprintf(out, "%d: %X\n", n + 1, c->u);
		}
		return 0;
	}
	int ret = sscanf(arg, "%hhu %X", &n, &u);
	if (!ret || n > conf.levels) return command("help color");
	if (!n) n = 1;
	c = &conf.thresh[n - 1].pseudo;
	if (ret == 1) c->a = 0;
	c->u = u;
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

int cmd_data(const char *arg) {
	GET_FOCUSED_IMG
	int n;
	if (!arg) note_write_file(NULL);
	else if ( (n=atoi(arg)) ) {
		note_set_current(focused_img, n);
		img_draw(focused_img);
		XFlush(dpy);
	}
	else command("help data");
	return 0;
}

int cmd_help(const char *arg) {
	Command *cmd = commands;
	if (!arg) {
		fprintf(out,
/* TODO fix these: */
"\n" VERSION_STRING
"\n\033[1mIMAGE WINDOW\033[0m\n"
"  \033[1mMouse Left Button\033[0m\n"
"    Drag to raise / move an image\n"
"  \033[1mMouse Right Button\033[0m\n"
"    Poly mode: Add points for a region selection\n"
"    Draw mode: Draw\n"
"    Note mode: Create new note mark\n"
"  \033[1mMouse Middle Button\033[0m\n"
"    Poly mode: Close a current region selection\n"
"    Draw mode: Toggle visibility of highlight overlay\n"
"    Note mode: Toggle visibility of note marks\n"
"\n\033[1mCOMMAND WINDOW\033[0m\n"
		);
	}
	for (cmd = commands; cmd->name; cmd++) {
		if (arg && !strncasecmp(arg, cmd->name, 3)) {
			if (cmd->detail) fprintf(out, "\n\033[1m%s\033[0m    %s\n",
					cmd->name, cmd->detail);
			else fprintf(out, "\n\033[1m%s\033[0m\n\n* %s\n",
					cmd->name, cmd->help);
			return 0;
		}
		else if (!arg && cmd->help) {
			fprintf(out, "  \033[1m%s\033[0m\n    %s\n", cmd->name, cmd->help);
		}
	}
	if (!arg) {
		fprintf(out,"\nUse \"help <command>\" for details on any command\n");
	}
}

int cmd_info(const char *arg) {
	GET_FOCUSED_IMG
	Img *img = focused_img;
	int i,j;
	double x1, x2, y1, y2;
	unsigned char *source_ptr, *sptr;
	int source_stride;
	Bool check;
	Col min, avg, max;
	unsigned long long nR = 0UL, nG = 0UL, nB = 0UL;
	int n = 0;
	cairo_clip_preserve(img->ctx);
	cairo_clip_extents(img->ctx, &x1, &y1, &x2, &y2);
	cairo_surface_flush(img->source);
	source_stride = cairo_image_surface_get_stride(img->source);
	source_ptr = cairo_image_surface_get_data(img->source);
	min.u = 0x00FFFFFF;
	avg.u = 0x00000000;
	max.u = 0x00000000;
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) check = False;
	else check = True;
	for (i = 0; i < img->w; i++) {
		for (j = 0; j < img->h; j++) {
			if (check && !cairo_in_clip(img->ctx, i, j)) continue;
			sptr = source_ptr + j * source_stride + i * 4;
			if (min.b > sptr[0]) min.b = sptr[0];
			if (min.g > sptr[1]) min.g = sptr[1];
			if (min.r > sptr[2]) min.r = sptr[2];
			if (min.b < sptr[0]) max.b = sptr[0];
			if (min.g < sptr[1]) max.g = sptr[1];
			if (min.r < sptr[2]) max.r = sptr[2];
			nB += sptr[0];
			nG += sptr[1];
			nR += sptr[2];
			n += 1;
		}
	}
	avg.r = nR / (float) n;
	avg.g = nG / (float) n;
	avg.b = nB / (float) n;
	fprintf(out, "     RRGGBB \nMin: %06X\nAvg: %06X\nMax: %06X\n", min.u, avg.u, max.u);
}

int cmd_layer(const char *arg) {
	if (!arg) conf.layers = !conf.layers;
	else switch (arg[0]) {
		case 'n': case 'f': conf.layers = False; break;
		default: conf.layers = True; break;
	}
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

int cmd_list(const char *arg) {
	Img *img;
	int i = 1;
	for (img = imgs; img; img = img->next, ++i) {
		if (focused_img == img) fprintf(out, "*%d: %s\n", i, img->name);
		else fprintf(out, " %d: %s\n", i, img->name);
	}
	return 0;
}

int cmd_mouse(const char *arg) {
	if (!arg) {
		if (conf.draw.a == MODE_DRAW) fprintf(out, "draw %X %d\n", conf.draw.u, conf.width);
		else if (conf.draw.a == MODE_POLY) fprintf(out, "select polygon\n");
		else if (conf.draw.a == MODE_NOTE) fprintf(out, "select a point for a label\n");
	}
	else if (arg[0] == 'd') {
		sscanf(arg, "%*s %X %d", &conf.draw.u, &conf.width);
		conf.draw.a = MODE_DRAW;
		if (!conf.width) conf.width = 2;
	}
	else if (arg[0] == 'p') conf.draw.a = MODE_POLY;
	else if (arg[0] == 'n') conf.draw.a = MODE_NOTE;
	img_draw(focused_img); // TODO: NEEDED?
	XFlush(dpy);
}

int cmd_move(const char *arg) {
	GET_FOCUSED_IMG
	if (!arg) return fprintf(out,"LOCATION: %d, %d\n", focused_img->x, focused_img->y);
	focused_img->x = focused_img->y = 0;
	sscanf(arg,"%d %d", &focused_img->x, &focused_img->y);
	XMoveWindow(dpy, focused_img->win, focused_img->x, focused_img->y);
	XFlush(dpy);
	return 0;
}

int cmd_name(const char *arg) {
	GET_FOCUSED_IMG
	fprintf(out,"NAME: %s\n", focused_img->name);
	return 0;
}

int cmd_note(const char *arg) {
	if (arg) dat_printf("note", "%s", arg);
	else dat_printf("note", "");
	return 0;
}

int cmd_open(const char *arg) {
	if (!arg) return 1;
	if (image_load(arg) != 0) {
		fprintf(out, "ERROR: Failed to open \"%s\"\n", arg);
		return 1;
	}
	return 0;
}

int cmd_poly(const char *arg) {
	GET_FOCUSED_IMG
	Img *img = focused_img;
	int x, y;
	char *tok, *sptr, *str;
	str = strdup(arg);
	cairo_new_sub_path(img->ctx);
	Col *c = &conf.line;
	cairo_set_source_rgba(img->ctx, c->r/255.0, c->g/255.0, c->b/255.0, c->a);
	for (tok=strtok_r(str," \t",&sptr); tok; tok=strtok_r(NULL," \t",&sptr)) {
		if (sscanf(tok, "%d,%d", &x, &y) == 2)
			cairo_line_to(img->ctx, x, y);
	}
	cairo_close_path(img->ctx);
	cairo_stroke(img->ctx);
	free(str);
	return 0;
}

int cmd_ratio(const char *arg) {
	long total;
	long *ret = _calculate(&total);
	uint8_t n, n1, n2;
	Col *c;
	if (!arg || arg[0] == 'r') {
		long sum = 0;
		for (n = 0; n < conf.levels; ++n) sum += ret[n];
		for (n = 0; n < conf.levels; ++n) {
			c = &conf.thresh[n].pseudo;
			if (arg) fprintf(out, "%d: %Lf\n", n + 1, ret[n] / (long double) sum);
			else fprintf(out, "%d: %Lf\n", n + 1, ret[n] / (long double) total);
		}
	}
	else if (sscanf(arg, "%hhu %hhu", &n1, &n2) == 2 &&
			n1 <= conf.levels && n2 <= conf.levels)
		dat_printf("ratio", "%Lf", ret[n1-1] / (long double) ret[n2-1]);
	else if (sscanf(arg, "%hhu", &n1) == 1 && n1 <= conf.levels)
		dat_printf("ratio", "%Lf", ret[n1-1] / (long double) total);
	else
		command("help ratio");
	free(ret);
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
		perror("[MAGOO]");
		out = stdout;
		return 0;
	}
	out = new;
}

int cmd_stretch(const char *arg) {
	GET_FOCUSED_IMG
	int n;
	if (!arg || !(n=atoi(arg))) return command("help stretch");
	Col *low = &conf.thresh[n - 1].low;
	Col *hi = &conf.thresh[n - 1].hi;
	img_stretch(focused_img, low, hi);
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
}

int cmd_threshold(const char *arg) {
	GET_FOCUSED_IMG
	Col *low, *hi;
	uint8_t n, r, g, b, ret;
	if (!arg) {
		for (n = 0; n < conf.levels; ++n) {
			low = &conf.thresh[n].low;
			hi = &conf.thresh[n].hi;
			fprintf(out, "%d: %06X -> %06X\n", n + 1, low->u, hi->u);
		}
		return;
	}
	ret = sscanf(arg, "%hhu", &n);
	if (!ret || (--n) >= conf.levels) return command("help threshold");
	low = &conf.thresh[n].low;
	hi = &conf.thresh[n].hi;
	ret = sscanf(arg, "%*hhu %X %X", &low->u, &hi->u);
	img_threshold_draw(focused_img);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

int cmd_zoom(const char *arg) {
	GET_FOCUSED_IMG
	float scale = focused_img->scale;
	if (!arg) return fprintf(out,"ZOOM: %f\n", scale);
	else if (arg[0] == 'u' || arg[0] == 'i') scale += 0.1;
	else if (arg[0] == 'd' || arg[0] == 'o') scale -= 0.1;
	else scale = atof(arg);
	if (scale < 0.05) scale = 0.05;
	else if (scale > 20.0) scale = 20.0;
	focused_img->scale = scale;
	img_resize(focused_img);
	cairo_identity_matrix(focused_img->ctx);
	cairo_scale(focused_img->ctx, scale, scale);
	cairo_set_line_width(focused_img->ctx, 1.25 / scale);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

