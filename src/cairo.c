/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

int img_threshold_draw(Img *img) {
	cairo_surface_flush(img->source);
	uint8_t *src, *sx, *dx, *dest;
	int i, j, src_stride, dest_stride;
	Col *low, *hi;
	int i0 = 0, j0 = 0, iN = img->w, jN = img->h;
	uint8_t n, full;
	src_stride = cairo_image_surface_get_stride(img->source);
	src = cairo_image_surface_get_data(img->source);
	for (n = 0; n < conf.levels; ++n) {
		cairo_surface_flush(img->mask[n]);
		dest_stride = cairo_image_surface_get_stride(img->mask[n]);
		dest = cairo_image_surface_get_data(img->mask[n]);
		low = &conf.thresh[n].low;
		hi = &conf.thresh[n].hi;
		full = conf.thresh[n].pseudo.a;
		if (full == 0) full = 255;
		for (j = j0; j < jN; j++) {
			for (i = i0; i < iN; i++) {
				sx = src + j*src_stride + i*4; // 4 bytes per pixel
				dx = dest + j * dest_stride + i;
				*dx = 0;
				if (sx[0] >= low->b && sx[0] <= hi->b &&
						sx[1] >= low->g && sx[1] <= hi->g &&
						sx[2] >= low->r && sx[2] <= hi->r)
					//*dx = 255;
					*dx = full;
			}
		}
		cairo_surface_mark_dirty(img->mask[n]);
	}
}

int img_stretch(Img *img, Col *low, Col *hi) {
	cairo_surface_flush(img->source);
	uint8_t *src, *sx;
	int i, j, src_stride;
	int i0 = 0, j0 = 0, iN = img->w, jN = img->h;
	src_stride = cairo_image_surface_get_stride(img->source);
	src = cairo_image_surface_get_data(img->source);
	double scale_r = 255 / (double) (hi->r - low->r);
	double scale_g = 255 / (double) (hi->g - low->g);
	double scale_b = 255 / (double) (hi->b - low->b);
	double val;
	for (j = j0; j < jN; j++) {
		for (i = i0; i < iN; i++) {
			sx = src + j*src_stride + i*4; // 4 bytes per pixel
			val = (sx[0] - low->b) * scale_b;
			//val = (sx[0] - low->b);
			sx[0] = (uint8_t) (val > 255 ? 255 : (val < 0 ? 0 : val));
			val = (sx[1] - low->g) * scale_g;
			//val = (sx[1] - low->g);
			sx[1] = (uint8_t) (val > 255 ? 255 : (val < 0 ? 0 : val));
			val = (sx[2] - low->r) * scale_r;
			//val = (sx[2] - low->r);
			sx[2] = (uint8_t) (val > 255 ? 255 : (val < 0 ? 0 : val));
		}
	}
	cairo_surface_flush(img->source);
}

static int cairo_helper_image_loader(const char *fname, Img *img) {
	GdkPixbuf *gpix;
	GError *gerr = NULL;
	if ( !(gpix=gdk_pixbuf_new_from_file(fname, &gerr)) ) {
		fprintf(stderr,"GDK Pixbuf Error\n");
		return 1;
	}
	gdk_pixbuf_get_file_info(fname, &img->w, &img->h);
	img->source = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		img->w, img->h);
	cairo_t *ctx = cairo_create(img->source);
	gdk_cairo_set_source_pixbuf(ctx, gpix, 0, 0);
	cairo_paint(ctx);
	cairo_destroy(ctx);
	g_object_unref(gpix);
	return 0;
}

Img *cairo_image_init(const char *fname) {
	Img *img;
	img = calloc(1, sizeof(Img));
	img->mask = calloc(conf.levels, sizeof(cairo_surface_t *));
	if (cairo_helper_image_loader(fname, img) != 0) {
		free(img);
		return NULL;
	}
	uint8_t n;
	for (n = 0; n < conf.levels; ++n)
		img->mask[n] = cairo_image_surface_create(CAIRO_FORMAT_A8, img->w, img->h);
	img->alpha = conf.alpha;
	img->scale = 1.0;
	img_threshold_draw(img);
	const char *name = strrchr(fname, '/');
	if (name) name++;
	else name = fname;
	img->name = strdup(name);
	char *dot = strrchr(img->name, '.');
	if (dot) *dot = '\0';
	xlib_image_init(img);
	cairo_surface_t *t;
	t = cairo_xlib_surface_create(dpy, img->win, vis, img->w, img->h);
	img->ctx = cairo_create(t);
	img->show_notes = True;
	cairo_surface_destroy(t);
	/* get data filename */
	dot = strrchr(fname, '.');
	if (dot) *dot = '\0';
	img->notes_file = malloc((strlen(fname) + 5) * sizeof(char));
	strcpy(img->notes_file, fname);
	strcat(img->notes_file, ".csv");
	/* if no file read, set default columns */
	if (note_read_file(img)) {
		for (n = 0; conf.columns[n]; ++n)
			img->columns[n] = conf.columns[n];
	}
	return img;
}

int cairo_image_free(Img *img) {
	note_write_file(img);
	uint8_t i, j;
	for (i = 0; i < conf.levels; ++i)
		cairo_surface_destroy(img->mask[i]);
	free_notes(img);
	free(img->note);
	free(img->mask);
	cairo_destroy(img->ctx);
	cairo_surface_destroy(img->source);
	free(img->notes_file);
	free(img->name);
	xlib_image_free(img);
	free(img);
	return 0;
}

int img_draw(Img *img) {
	cairo_set_source_rgba(img->ctx, 1, 1, 1, 1);
	cairo_paint(img->ctx);
	cairo_set_source_surface(img->ctx, img->source, 0, 0);
	cairo_paint_with_alpha(img->ctx, img->alpha / 255.0);
	double x1, x2, y1, y2;
	cairo_path_extents(img->ctx, &x1, &y1, &x2, &y2);
	if (!(x1 == 0) || !(y1 == 0) || !(x2 == 0) || !(y2 == 0))
		cairo_clip_preserve(img->ctx);
	Col *c;
	uint8_t n;
	if (conf.layers) {
		for (n = 0; n < conf.levels; ++n) {
			cairo_set_source_surface(img->ctx, img->source, 0, 0);
			c = &conf.thresh[n].pseudo;
			if (c->a) cairo_set_source_rgba(img->ctx,
					c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a);
			cairo_mask_surface(img->ctx, img->mask[n], 0, 0);
		}
	}
	cairo_reset_clip(img->ctx);
	c = &conf.line;
	cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a);
	cairo_stroke_preserve(img->ctx);
if (img->show_notes) {
int x, y;
char str[5];
cairo_text_extents_t ext;
	for (n = 0; n < img->nnotes; ++n) {
x = img->note[n].x;
y = img->note[n].y;
snprintf(str,4, "%d", n+1);
cairo_text_extents(img->ctx, str, &ext);
cairo_rectangle(img->ctx,
	x - 4 + ext.x_bearing,
	y - 4 + ext.y_bearing,
	ext.width + 8,
	ext.height + 8);
		if (n == img->curnote - 1) c = &conf.bgCurNote;
		else c = &conf.bgNote;
		cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a / 255.0);
cairo_fill_preserve(img->ctx);
		if (n == img->curnote - 1) c = &conf.fgCurNote;
		else c = &conf.fgNote;
		cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a / 255.0);
cairo_stroke(img->ctx);
		cairo_set_source_rgba(img->ctx, 0, 0, 0.5, 0.9);
		cairo_move_to(img->ctx, x, y);
		cairo_show_text(img->ctx, str);
	}
}
	xlib_draw();
	return 0;
}

int img_resize(Img *img) {
	cairo_surface_t *t;
	int w = img->w * img->scale;
	int h = img->h * img->scale;
	XResizeWindow(dpy, img->win, w, h);
	t = cairo_xlib_surface_create(dpy, img->win, vis, w, h);
	cairo_destroy(img->ctx);
	img->ctx = cairo_create(t);
	cairo_surface_destroy(t);
	return 0;
}

