/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

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
static int cmd_echo(const char *);
static int cmd_help(const char *);
static int cmd_info(const char *);
static int cmd_list(const char *);
static int cmd_move(const char *);
static int cmd_name(const char *);
static int cmd_open(const char *);
static int cmd_quit(const char *);
static int cmd_ratio(const char *);
static int cmd_shell(const char *);
static int cmd_sink(const char *);
static int cmd_threshold(const char *);
static int cmd_zoom(const char *);

//static unsigned char *data_ptr1, *data_ptr2, *dptr;
//static int data_stride1, data_stride2;
//static Col *col1_low, *col1_hi, *col2_low, *col2_hi;
//static Bool check_crop_area;
static FILE *out = NULL;
Command _commands[] = {
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
		if (!strncasecmp(s, cmd->name, 3))
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
	char *clean;
	if (strncmp(fname, "file://", 7) == 0) clean = strdup(fname + 7);
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

static long _calculate(long *ret) {
	GET_FOCUSED_IMG
	Img *img = focused_img;
	int i, j, n;
	double x1, x2, y1, y2;
	long area = 0;
	cairo_clip_preserve(img->ctx);
	cairo_clip_extents(img->ctx, &x1, &y1, &x2, &y2);
	uint8_t *dptr[NTHRESH];
	int dstride[NTHRESH];
	for (n = 0; n < NTHRESH; ++n) {
		cairo_surface_flush(img->thresh[n].pix);
		dptr[n] = cairo_image_surface_get_data(img->thresh[n].pix);
		dstride[n] = cairo_image_surface_get_stride(img->thresh[n].pix);
		ret[n] = 0;
	}
	Bool crop = True;
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) crop = False;
	for (i = 0; i < img->w; i++) {
		for (j = 0; j < img->h; j++) {
			if (crop && !cairo_in_clip(img->ctx, i, j)) continue;
			for (n = 0; n < NTHRESH; ++n)
				ret[n] += ( *(dptr[n] + j * dstride[n] + i) == 255 ? 1 : 0);
			area += 1;
		}
	}
	cairo_reset_clip(img->ctx);
	return area;
}

/****************************\
|* Command functions
\****************************/

int cmd_alpha(const char *arg) {
	GET_FOCUSED_IMG
	if (!arg) fprintf(out, "%03d\n", focused_img->source.alpha);
	else {
		sscanf(arg, "%hhu", &focused_img->source.alpha);
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
	long ret[NTHRESH];
	long total = _calculate(ret);
	uint8_t n;
	if (!arg) {
		for (n = 0; n < NTHRESH; ++n)
			fprintf(out, "%d: %ld\n", n + 1, ret[n]);
		fprintf(out, "Total: %ld\n", total);
	}
	else if (arg[0] == 't') fprintf(out, "%ld\n", total);
	else if (arg[0] == 'a') fprintf(out, "%ld\n", total);
	else if ( (n=atoi(arg)) ) fprintf(out, "%ld\n", ret[n-1]);
	else command("help count");
	return 0;
}

int cmd_color(const char *arg) {
	GET_FOCUSED_IMG
	Col *c;
	unsigned short int n, r, g, b, a;
	if (!arg) {
		fprintf(out, "RGBA:\n");
		for (n = 0; n < NTHRESH; ++n) {
			c = &focused_img->thresh[n].pseudo;
			fprintf(out, "%d: %03d %03d %03d %03d\n", n + 1, c->r, c->g, c->b, c->a);
		}
		return 0;
	}
	int ret = sscanf(arg, "%hhu %hhu %hhu %hhu %hhu", &n, &r, &g, &b, &a);
	if (!ret || n > NTHRESH) return command("help color");
	if (!n) n = 1;
	c = &focused_img->thresh[n - 1].pseudo;
	if (ret == 1) c->a = 0;
	if (ret < 5) a = 255;
	c->r = r; c->g = g; c->b = b; c->a = a;
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
	if (!arg) {
		fprintf(out,
"\n" VERSION_STRING
"\n\033[1mIMAGE WINDOW\033[0m\n"
"  \033[1mMouse Left Button\033[0m\n"
"    Drag to raise / move an image\n"
"  \033[1mMouse Right Button\033[0m\n"
"    Click to add points for a region selection\n"
"  \033[1mMouse Middle Button\033[0m\n"
"    Close a current region selection\n"
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
	Col low, hi;
	unsigned long long nR = 0L, nG = 0L, nB = 0L;
	int n = 0;
	cairo_clip_preserve(img->ctx);
	cairo_clip_extents(img->ctx, &x1, &y1, &x2, &y2);
	cairo_surface_flush(img->source.pix);
	source_stride = cairo_image_surface_get_stride(img->source.pix);
	source_ptr = cairo_image_surface_get_data(img->source.pix);
	low = (Col) {255, 255, 255};
	hi = (Col) {0, 0, 0};
	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) check = False;
	else check = True;
	for (i = 0; i < img->w; i++) {
		for (j = 0; j < img->h; j++) {
			if (check && !cairo_in_clip(img->ctx, i, j)) continue;
			sptr = source_ptr + j * source_stride + i * 4;
			if (low.b > sptr[0]) low.b = sptr[0];
			if (low.g > sptr[1]) low.g = sptr[1];
			if (low.r > sptr[2]) low.r = sptr[2];
			if (hi.b < sptr[0]) hi.b = sptr[0];
			if (hi.g < sptr[1]) hi.g = sptr[1];
			if (hi.r < sptr[2]) hi.r = sptr[2];
			nB += sptr[0];
			nG += sptr[1];
			nR += sptr[2];
			n += 1;
		}
	}
	fprintf(out, "     R   G   B \nLO: %03d %03d %03d\nHI: %03d %03d %03d\n"
			"AVG %03d %03d %03d\n", low.r, low.g, low.b, hi.r, hi.g, hi.b,
			(int) (nR / (float) n), (int) (nG / (float) n), (int) (nB / (float) n));
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
	fprintf(out,"NAME: %s\n", focused_img->source.name);
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

int cmd_ratio(const char *arg) {
	long ret[NTHRESH];
	long total = _calculate(ret);
	uint8_t n, n1, n2;
	Col *c;
	if (!arg || arg[0] == 'r') {
		long sum = 0;
		for (n = 0; n < NTHRESH; ++n) sum += ret[n];
		for (n = 0; n < NTHRESH; ++n) {
			c = &focused_img->thresh[n].pseudo;
			if (arg) fprintf(out, "%d: %Lf\n", n + 1, ret[n] / (long double) sum);
			else fprintf(out, "%d: %Lf\n", n + 1, ret[n] / (long double) total);
		}
	}
	else if (arg[0] == 'g')
		fprintf(out, "%Lf\n", ret[0] / (long double) (ret[0] + ret[1]));
	else if (sscanf(arg, "%hhu %hhu", &n1, &n2) == 2 && n1 <= NTHRESH && n2 <= NTHRESH )
		fprintf(out, "%Lf\n", ret[n1-1] / (long double) ret[n2-1]);
	else
		command("help ratio");
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

int cmd_threshold(const char *arg) {
	GET_FOCUSED_IMG
	Col *low, *hi;
	uint8_t n, r, g, b, ret;
	if (!arg) {
		fprintf(out, "RGB   LOW | HI\n");
		for (n = 0; n < NTHRESH; ++n) {
			low = &focused_img->thresh[n].low;
			hi = &focused_img->thresh[n].hi;
			fprintf(out, "%d: %03d %03d %03d | ", n + 1, low->r, low->g, low->b);
			fprintf(out, "%03d %03d %03d\n", hi->r, hi->g, hi->b);
		}
		return;
	}
	char spec[64];
	ret = sscanf(arg, "%hhu %s %hhu %hhu %hhu", &n, spec, &r, &g, &b);
	if (ret < 3 || n > NTHRESH) return command("help threshold");
	if (!n) n = 1;
	low = &focused_img->thresh[n - 1].low;
	hi = &focused_img->thresh[n - 1].hi;
	switch (spec[0]) {
		case 'r': case 'R': low->r = r; hi->r = g; break;
		case 'g': case 'G': low->g = r; hi->g = g; break;
		case 'b': case 'B': low->b = r; hi->b = g; break;
		case 'l': case 'L': low->r = r; low->g = g; low->b = b; break;
		case 'h': case 'H': hi->r = r; hi->g = g; hi->b = b; break;
		default: return command("help threshold");
	}
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
	if (scale < 0.1) scale = 0.1;
	else if (scale > 1.0) scale = 1.0;
	cairo_identity_matrix(focused_img->ctx);
	cairo_scale(focused_img->ctx, scale, scale);
	cairo_set_line_width(focused_img->ctx, 1.25 / scale);
	focused_img->scale = scale;
	XResizeWindow(dpy, focused_img->win, focused_img->w * scale, focused_img->h * scale);
	img_draw(focused_img);
	XFlush(dpy);
	return 0;
}

