/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

int img_threshold_draw(Img *img) {
	cairo_surface_flush(img->source.pix);
	unsigned char *src, *dest[NTHRESH], *sx, *dx;
	int i, j, src_stride, dest_stride[NTHRESH];
	Col *low[NTHRESH], *hi[NTHRESH];
	int i0 = 0, j0 = 0, iN = img->w, jN = img->h;
	uint8_t n;
	src_stride = cairo_image_surface_get_stride(img->source.pix);
	src = cairo_image_surface_get_data(img->source.pix);
	for (n = 0; n < NTHRESH; ++n) {
		cairo_surface_flush(img->thresh[n].pix);
		dest_stride[n] = cairo_image_surface_get_stride(img->thresh[n].pix);
		dest[n] = cairo_image_surface_get_data(img->thresh[n].pix);
		//low[n] = &img->thresh[n].low;
		//hi[n] = &img->thresh[n].hi;
low[n] = &conf.thresh[n].low;
hi[n] = &conf.thresh[n].hi;
	}
	for (j = j0; j < jN; j++) {
		for (i = i0; i < iN; i++) {
			sx = src + j*src_stride + i*4; // 4 bytes per pixel
			for (n = 0; n < NTHRESH; ++n) {
				dx = dest[n] + j * dest_stride[n] + i;
				*dx = 0;
				if (sx[0] >= low[n]->b && sx[0] <= hi[n]->b &&
						sx[1] >= low[n]->g && sx[1] <= hi[n]->g &&
						sx[2] >= low[n]->r && sx[2] <= hi[n]->r)
					*dx = 255;
			}
		}
	}
	for (n = 0; n < NTHRESH; ++n)
		cairo_surface_mark_dirty(img->thresh[n].pix);
}

static int cairo_helper_image_loader(const char *fname, Img *img) {
	GdkPixbuf *gpix;
	GError *gerr = NULL;
	if ( !(gpix=gdk_pixbuf_new_from_file(fname, &gerr)) ) {
		fprintf(stderr,"GDK Pixbuf Error\n");
		return 1;
	}
	gdk_pixbuf_get_file_info(fname, &img->w, &img->h);
	img->source.pix = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		img->w, img->h);
	cairo_t *ctx = cairo_create(img->source.pix);
	gdk_cairo_set_source_pixbuf(ctx, gpix, 0, 0);
	cairo_paint(ctx);
	cairo_destroy(ctx);
	g_object_unref(gpix);
	return 0;
}

Img *cairo_image_init(const char *fname) {
	Img *img;
	img = calloc(1, sizeof(Img));
	if (cairo_helper_image_loader(fname, img) != 0) {
		free(img);
		return NULL;
	}
	uint8_t n;
	for (n = 0; n < NTHRESH; ++n) {
		img->thresh[n].pix = cairo_image_surface_create(CAIRO_FORMAT_A8, img->w, img->h);
//		img->thresh[n].low.r = conf.thresh[n].low.r;
//		img->thresh[n].low.g = conf.thresh[n].low.g;
//		img->thresh[n].low.b = conf.thresh[n].low.b;
//		img->thresh[n].hi.r = conf.thresh[n].hi.r;
//		img->thresh[n].hi.g = conf.thresh[n].hi.g;
//		img->thresh[n].hi.b = conf.thresh[n].hi.b;
//		img->thresh[n].pseudo = conf.thresh[n].pseudo;
	}
	img->crop.line = conf.line;
	img->source.alpha = conf.alpha;
	img->scale = 1.0;
	img_threshold_draw(img);
	const char *name = strrchr(fname, '/');
	if (name) name++;
	else name = fname;
	img->source.name = strdup(name);
	char *dot = strrchr(img->source.name, '.');
	if (dot) *dot = '\0';
	xlib_image_init(img);
	cairo_surface_t *t;
	t = cairo_xlib_surface_create(dpy, img->win, vis, img->w, img->h);
	img->ctx = cairo_create(t);
	cairo_surface_destroy(t);
	return img;
}

int cairo_image_free(Img *img) {
	uint8_t n;
	for (n = 0; n < NTHRESH; ++n)
		cairo_surface_destroy(img->thresh[n].pix);
	cairo_destroy(img->ctx);
	cairo_surface_destroy(img->source.pix);
	free(img->source.name);
	xlib_image_free(img);
	free(img);
	return 0;
}

int img_draw(Img *img) {
	cairo_set_source_rgba(img->ctx, 1, 1, 1, 1);
	cairo_paint(img->ctx);
	cairo_set_source_surface(img->ctx, img->source.pix, 0, 0);
	cairo_paint_with_alpha(img->ctx, img->source.alpha / 255.0);

	double x1, x2, y1, y2;
	cairo_path_extents(img->ctx, &x1, &y1, &x2, &y2);
	if (!(x1 == 0) || !(y1 == 0) || !(x2 == 0) || !(y2 == 0))
		cairo_clip_preserve(img->ctx);
	Col *c;
	uint8_t n;
	for (n = 0; n < NTHRESH; ++n) {
		cairo_set_source_surface(img->ctx, img->source.pix, 0, 0);
		//c = &img->thresh[n].pseudo;
c = &conf.thresh[n].pseudo;
		if (c->a)
			cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a);
		cairo_mask_surface(img->ctx, img->thresh[n].pix, 0, 0);
	}
	cairo_reset_clip(img->ctx);
	c = &img->crop.line;
	cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a);
	cairo_stroke_preserve(img->ctx);
	return 0;
}
