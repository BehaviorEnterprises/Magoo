/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"
#include "math.h"

static void buttonpress(XEvent *);
static void clientmessage(XEvent *);
static void expose(XEvent *);
static void keypress(XEvent *);
static void propertynotify(XEvent *);

static Atom dnd;

static Window root, win;
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]     = buttonpress,
	[ClientMessage]	= clientmessage,
	[Expose]          = expose,
	[KeyPress]        = keypress,
	[PropertyNotify]  = propertynotify,
};

int main_loop() {
	/* map all windows: */
	XMapWindow(dpy, win);
	Img *img;
	for (img = imgs; img; img = img->next) XMapWindow(dpy, img->win);
	XFlush(dpy);
	/* set up loop: */
	running = True;
	XEvent ev;
	fd_set fds;
	int xfd = ConnectionNumber(dpy);
	int max = (xfd > input ? xfd + 1 : input + 1);
	char cmd[256];
	kill(terminal, SIGUSR1);
	/* loop: */
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

int xlib_image_init(Img *img) {
	XSetWindowAttributes wa;
	wa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask;
	wa.backing_store = Always;
	img->win = XCreateWindow(dpy, win, img->x, img->y, img->w, img->h, 1, dep,
			InputOutput, vis, CWEventMask | CWBackingStore, &wa);
	return 0;
}

int xlib_image_free(Img *img) {
	XDestroyWindow(dpy, img->win);
	XFlush(dpy);
	return 0;
}

int xlib_init() {
	/* connect to display and get defaults */
	if (!(dpy=XOpenDisplay(0x0))) {
		fprintf(stderr, "Failed to open display");
		exit(1);
	}
dnd = XInternAtom(dpy, "XdndAware", False);
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
	XStoreName(dpy, win, "Magoo: Images");
Atom atom = XInternAtom(dpy, "ATOM", False);
Atom fname = XInternAtom(dpy, "FILENAME", False);
XChangeProperty(dpy, win, dnd, atom, 32, PropModeReplace, (unsigned char *)&fname, 1);
	// TODO create cursors : crosshair ...
	XFlush(dpy);
	return 0;
}

int xlib_free() {
	while (imgs) image_unload(imgs);
	XDestroyWindow(dpy, win);
	XSync(dpy,True);
	XCloseDisplay(dpy);
	return 0;
}

void buttonpress(XEvent *ev) {
	static Bool path_open = False;
	XButtonEvent *e = &ev->xbutton;
	Img *img;
	for (img = imgs; img; img = img->next)
		if (e->window == img->win) break;
	if (!img) return;
	if (e->button == 1) {
		XRaiseWindow(dpy, img->win);
		focused_img = img;
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
		path_open = False;
	}
	else if (e->button == 3) {
		if (!path_open) cairo_arc(img->ctx, e->x, e->y, 1, 0, 2*M_PI);
		cairo_line_to(img->ctx, e->x / img->scale, e->y / img->scale);
		Col *c = &img->crop.line;
		cairo_set_source_rgba(img->ctx, c->r/255.0, c->g/255.0, c->b/255.0, c->a);
		cairo_stroke_preserve(img->ctx);
		XFlush(dpy);
		path_open = True;
	}
}

void clientmessage(XEvent *ev) {
	fprintf(stderr, "Client Message\n");
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
