/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

int img_threshold_draw(Img *img) {
	cairo_surface_flush(img->source.pix);
	cairo_surface_flush(img->threshold.pix);
	unsigned char *src, *dest, *sx, *dx;
	int i, j, src_stride, dest_stride;
	int i0 = 0, j0 = 0, iN = img->w, jN = img->h;
	src_stride = cairo_image_surface_get_stride(img->source.pix);
	dest_stride = cairo_image_surface_get_stride(img->threshold.pix);
	src = cairo_image_surface_get_data(img->source.pix);
	dest = cairo_image_surface_get_data(img->threshold.pix);
	Col *low, *hi;
	low = &img->threshold.low;
	hi = &img->threshold.hi;
	for (j = j0; j < jN; j++) {
		for (i = i0; i < iN; i++) {
			sx = src + j*src_stride + i*4; // 4 bytes per pixel
			dx = dest + j*dest_stride + i;
			if (sx[0] >= low->b && sx[0] <= hi->b &&
					sx[1] >= low->g && sx[1] <= hi->g &&
					sx[2] >= low->r && sx[2] <= hi->r)
				*dx = 255;
			else *dx = 0;
		}
	}
	cairo_surface_mark_dirty(img->threshold.pix);
}

static int cairo_helper_image_loader(const char *fname, Img *img) {
	GdkPixbuf *gpix;
	GError *gerr = NULL;
	if ( !(gpix=gdk_pixbuf_new_from_file(fname, &gerr)) )
		die("gpix error\n");
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

Img *cairo_create_img(const char *fname) {
	Img *img;
	img = calloc(1, sizeof(Img));
	cairo_helper_image_loader(fname, img);
	img->threshold.pix = cairo_image_surface_create(CAIRO_FORMAT_A8,
			img->w, img->h);
	img->threshold.low.r = conf.threshold.low.r;
	img->threshold.low.g = conf.threshold.low.g;
	img->threshold.low.b = conf.threshold.low.b;
	img->threshold.hi.r = conf.threshold.hi.r;
	img->threshold.hi.g = conf.threshold.hi.g;
	img->threshold.hi.b = conf.threshold.hi.b;
	img->threshold.pseudo = conf.threshold.pseudo;
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
	img->next = imgs;
	imgs = img;
	return img;
}

int cairo_init_img(Img *img) {
	cairo_surface_t *t;
	t = cairo_xlib_surface_create(dpy, img->win, vis, img->w, img->h);
	img->ctx = cairo_create(t);
	cairo_surface_destroy(t);
}

int cairo_destroy_img(Img *img) {
	free(img);
	return 0;
}

int cairo_free_img(Img *img) {
	cairo_surface_destroy(img->threshold.pix);
	cairo_destroy(img->ctx);
	cairo_surface_destroy(img->source.pix);
	free(img->source.name);
	return 0;
}

int img_draw(Img *img) {
	cairo_set_source_rgba(img->ctx, 1, 1, 1, 1);
	cairo_paint(img->ctx);
	//cairo_save(img->ctx);
	//cairo_scale(img->ctx,img->scale,img->scale);
	cairo_set_source_surface(img->ctx, img->source.pix, 0, 0);
	cairo_paint_with_alpha(img->ctx, img->source.alpha / 255.0);
	Col *c = &img->threshold.pseudo;
	if (c->a) cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0,
			c->b / 255.0, c->a);
	double x1, x2, y1, y2;
	cairo_path_extents(img->ctx, &x1, &y1, &x2, &y2);
	if (!(x1 == 0) || !(y1 == 0) || !(x2 == 0) || !(y2 == 0))
	cairo_clip_preserve(img->ctx);
	cairo_mask_surface(img->ctx, img->threshold.pix, 0, 0);
	cairo_reset_clip(img->ctx);
	c = &img->crop.line;
	cairo_set_source_rgba(img->ctx, c->r / 255.0, c->g / 255.0, c->b / 255.0, c->a);
	cairo_stroke_preserve(img->ctx);
	//cairo_restore(img->ctx);
	return 0;
}
