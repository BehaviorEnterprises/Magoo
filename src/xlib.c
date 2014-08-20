/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

static void buttonpress(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void propertynotify(XEvent *);

static Window root, win;
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]     = buttonpress,
	[Expose]          = expose,
	[KeyPress]        = keypress,
	[PropertyNotify]  = propertynotify,
};

int main_loop() {
	running = True;
	XEvent ev;
	fd_set fds;
	int xfd = ConnectionNumber(dpy);
	int max = (xfd > input ? xfd + 1 : input + 1);
	char cmd[256];
	kill(terminal, SIGUSR1);
	while (running) {
		FD_ZERO(&fds);
		FD_SET(xfd, &fds);
		FD_SET(input, &fds);
		select(max, &fds, 0, 0, NULL);
		if (FD_ISSET(xfd, &fds)) {
			while (XPending(dpy)) {
				XNextEvent(dpy, &ev);
				if (ev.type < LASTEvent && handler[ev.type])
					handler[ev.type](&ev);
			}
		}
		if (FD_ISSET(input, &fds)) {
			read(input, cmd, sizeof(cmd));
			if (cmd[0] != '\0') command(cmd);
			if (!running) kill(terminal, SIGUSR2);
			kill(terminal, SIGUSR1);
		}
	}
	return 0;
}

static int xlib_create_img(Img *img) {
	XSetWindowAttributes wa;
	wa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask;
	wa.backing_store = Always;
	img->win = XCreateWindow(dpy, win, img->x, img->y, img->w, img->h, 1, dep,
			InputOutput, vis, CWEventMask | CWBackingStore, &wa);
	if (img->win) return cairo_init_img(img);
	else return 1;
}

static int xlib_destroy_img(Img *img) {
	cairo_free_img(img);
	XDestroyWindow(dpy, img->win);
	cairo_destroy_img(img);
	return 0;
}

int xlib_init() {
	/* connect to display and get defaults */
	if (!(dpy=XOpenDisplay(0x0))) die("Failed to open display");
	scr = DefaultScreen(dpy);
	dep = DefaultDepth(dpy, scr);
	vis = DefaultVisual(dpy, scr);
	gc = DefaultGC(dpy, scr);
	root = RootWindow(dpy, scr);
	/* create main window and tool windown */
	Pixmap pix;
	unsigned char bits[8] = { 136, 68, 34, 17, 8, 4, 2, 1 };
	//unsigned char bits[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
	pix = XCreatePixmapFromBitmapData(dpy, root, bits, 8, 8, 0xE0E0E0, 0xBBCCFF, dep);
	XSetWindowAttributes wa;
	wa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
			PropertyChangeMask | StructureNotifyMask;
	wa.background_pixel = 0xBBCCFF;
	wa.background_pixmap = pix;
	win = XCreateWindow(dpy, root, 0, 0, ww=640, wh=480, 0, dep,
			InputOutput, vis, CWEventMask | CWBackPixmap, &wa);
	XStoreName(dpy, win, "Scope");
	// TODO create cursors : crosshair ...
	XMapWindow(dpy, win);
	Img *img;
	for (img = imgs; img; img = img->next) {
		xlib_create_img(img);
		XMapWindow(dpy, img->win);
	}
	XFlush(dpy);
	return 0;
}

int xlib_free() {
	Img *img;
	while ( (img=imgs) ) {
		imgs = img->next;
		xlib_destroy_img(img);
	}
	XDestroyWindow(dpy, win);
	XFlush(dpy);
	XCloseDisplay(dpy);
	return 0;
}

void buttonpress(XEvent *ev) {
	XButtonEvent *e = &ev->xbutton;
	Img *img;
	for (img = imgs; img; img = img->next)
		if (e->window == img->win) break;
	if (!img) return;
	//int del = 8;
	//if (e->state == ShiftMask) del = 16;
	//else if (e->state == ControlMask) del = 4;
	if (e->button == 1) {
		XRaiseWindow(dpy, img->win);
		XGrabPointer(dpy, root, True, PointerMotionMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
		int xx = e->x_root, yy = e->y_root, dx, dy;
		XEvent ee;
		while (True) {
			XMaskEvent(dpy, PointerMotionMask | ButtonReleaseMask, &ee);
			if (ee.type == ButtonRelease) break;
			dx = ee.xbutton.x_root - xx; xx = ee.xbutton.x_root;
			dy = ee.xbutton.y_root - yy; yy = ee.xbutton.y_root;
			img->x += dx; img->y += dy;
			XMoveWindow(dpy, img->win, img->x, img->y);
		}
		XUngrabPointer(dpy, CurrentTime);
	}
	else if (e->button == 2) {
		cairo_close_path(img->ctx);
		cairo_new_sub_path(img->ctx);
		img_draw(img);
	}
	else if (e->button == 3) {
		cairo_line_to(img->ctx, e->x / img->scale, e->y / img->scale);
		cairo_set_source_rgba(img->ctx, 1.0, 0.5, 0.5, 1.0);
		cairo_stroke_preserve(img->ctx);
	}
}

void expose(XEvent *ev) {
	Img *img;
	for (img = imgs; img; img = img->next)
		if (ev->xexpose.window == img->win) img_draw(img);
}

void keypress(XEvent *ev) {
}

void propertynotify(XEvent *ev) {
}
