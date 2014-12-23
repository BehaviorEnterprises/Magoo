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
static void selectionnotify(XEvent *);

enum atom_type {
	FileType, FileName,
	XdndAware, XdndSelection, XdndTypeList,
	XdndActionCopy,
	XdndEnter, XdndLeave, XdndDrop, XdndFinished,
	XdndPosition, XdndStatus,
	WmProtocols, WmDeleteWindow,
	LASTAtom
};

static Atom atoms[LASTAtom];

static Window root, win;
static Cursor cursor_draw, cursor_poly, cursor_move;
static void (*handler[LASTEvent])(XEvent *) = {
	[ButtonPress]     = buttonpress,
	[ClientMessage]	= clientmessage,
	[Expose]          = expose,
	[KeyPress]        = keypress,
	[PropertyNotify]  = propertynotify,
	[SelectionNotify] = selectionnotify,
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
				else if (ev.type == UnmapNotify) {
					XMapWindow(dpy, win);
				}
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

int xlib_draw() {
	if (conf.draw.a == MODE_DRAW) XDefineCursor(dpy, win, cursor_draw);
	else XDefineCursor(dpy, win, cursor_poly);
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
	scr = DefaultScreen(dpy);
	dep = DefaultDepth(dpy, scr);
	vis = DefaultVisual(dpy, scr);
	gc = DefaultGC(dpy, scr);
	root = RootWindow(dpy, scr);
	atoms[FileType] = XInternAtom(dpy, "text/uri-list", False);
	atoms[FileName] = XInternAtom(dpy, "MagooFileName", False);
	atoms[XdndAware] = XInternAtom(dpy, "XdndAware", False);
	atoms[XdndSelection] = XInternAtom(dpy, "XdndSelection", False);
	atoms[XdndEnter] = XInternAtom(dpy, "XdndEnter", False);
	atoms[XdndTypeList] = XInternAtom(dpy, "XdndTypeList", False);
	atoms[XdndPosition] = XInternAtom(dpy, "XdndPosition", False);
	atoms[XdndStatus] = XInternAtom(dpy, "XdndStatus", False);
	atoms[XdndLeave] = XInternAtom(dpy, "XdndLeave", False);
	atoms[XdndDrop] = XInternAtom(dpy, "XdndDrop", False);
	atoms[XdndFinished] = XInternAtom(dpy, "XdndFinished", False);
	atoms[XdndActionCopy] = XInternAtom(dpy, "XdndActionCopy", False);
	atoms[WmProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	atoms[WmDeleteWindow] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
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
	unsigned char dndver = 5;
	XChangeProperty(dpy, win, atoms[XdndAware], XA_ATOM, 32, PropModeReplace, &dndver, 1);
	XSetWMProtocols(dpy, win, &atoms[WmDeleteWindow], 1);
	cursor_draw = XCreateFontCursor(dpy, 68);
	cursor_poly = XCreateFontCursor(dpy, 34);
	cursor_move = XCreateFontCursor(dpy, 52);
	XFlush(dpy);
	return 0;
}

int xlib_free() {
	while (imgs) image_unload(imgs);
	XFreeCursor(dpy, cursor_draw);
	XFreeCursor(dpy, cursor_poly);
	XFreeCursor(dpy, cursor_move);
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
		XDefineCursor(dpy, win, cursor_move);
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
		if (conf.draw.a == MODE_DRAW) XDefineCursor(dpy, win, cursor_draw);
		else XDefineCursor(dpy, win, cursor_poly);
	}
	else if (e->button == 2) {
		if (conf.draw.a == MODE_DRAW) command("layer");
		else if (conf.draw.a == MODE_POLY) {
			cairo_close_path(img->ctx);
			cairo_new_sub_path(img->ctx);
			img_draw(img);
			path_open = False;
		}
		else if (conf.draw.a == MODE_NOTE) {
			img->show_notes = !img->show_notes;
			img_draw(img);
		}
	}
	else if (e->button == 3) {
		if (conf.draw.a == MODE_DRAW) {
			XRaiseWindow(dpy, img->win);
			focused_img = img;
cairo_t *ctx = cairo_create(img->source);
Col *c = &conf.draw;
			XGrabPointer(dpy, img->win, True, PointerMotionMask | ButtonReleaseMask,
					GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
cairo_set_source_rgba(ctx, c->r / 255.0, c->g / 255.0,
		c->b / 255.0, 1.0);
cairo_set_line_width(ctx, conf.width);
cairo_move_to(ctx, e->x / img->scale, e->y / img->scale);
			XEvent ee;
			while (True) {
				XMaskEvent(dpy, PointerMotionMask | ButtonReleaseMask, &ee);
				if (ee.type == ButtonRelease) break;
cairo_line_to(ctx, ee.xbutton.x / img->scale, ee.xbutton.y / img->scale);
cairo_stroke_preserve(ctx);
img_draw(img);
			}
			XUngrabPointer(dpy, CurrentTime);
cairo_stroke(ctx);
img_threshold_draw(img);
img_draw(img);
		}
		else if (conf.draw.a == MODE_POLY) {
			if (!path_open) cairo_arc(img->ctx, e->x, e->y, 1, 0, 2*M_PI);
			cairo_line_to(img->ctx, e->x / img->scale, e->y / img->scale);
			Col *c = &conf.line;
			cairo_set_source_rgba(img->ctx, c->r/255.0, c->g/255.0, c->b/255.0, c->a);
			cairo_stroke_preserve(img->ctx);
			XFlush(dpy);
			path_open = True;
		}
		else if (conf.draw.a == MODE_NOTE) {
			note_create_label(img, (int) (e->x / img->scale), (int) (e->y / img->scale));
			img_draw(img);
		}
	}
}

/*
static void _send_msg(Window target, int type, msgdata ) {
	XEvent ev;
	ev.type = ClientMessage;
	ev.xclient.window = target;
	ev.xclient.message_type = atoms[type];
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = atoms[msg];
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dpy, target, False, NoEventMask, &ev);
}
*/

void clientmessage(XEvent *ev) {
	static Window source = None;
	XClientMessageEvent *e = &ev->xclient;
	Atom action;
	Window igw;
	XEvent msg;
	msg.type = ClientMessage;
	msg.xclient.format = 32;
	int wx, wy, ww, wh, ign;
	XGetGeometry(dpy, win, &igw, &wx, &wy, &ww, &wh, &ign, &ign);
	if (e->message_type == atoms[XdndEnter]) {
		source = e->data.l[0];
	}
	else if (e->message_type == atoms[XdndPosition]) {
		action = e->data.l[4];
		msg.xclient.message_type = atoms[XdndStatus];
		msg.xclient.window = source;
		msg.xclient.data.l[0] = win;
		msg.xclient.data.l[1] = 0x01;
		msg.xclient.data.l[2] = (wx << 16) | wy;
		msg.xclient.data.l[3] = (ww << 16) | wh;
		msg.xclient.data.l[4] = atoms[XdndActionCopy];
		XSendEvent(dpy, source, False, NoEventMask, &msg);
	}
	else if (e->message_type == atoms[XdndLeave]) {
		source = None;
	}
	else if (e->message_type == atoms[XdndDrop]) {
		XConvertSelection(dpy, atoms[XdndSelection], atoms[FileType],
			atoms[FileName], win, e->data.l[2]);
		msg.xclient.message_type = atoms[XdndFinished];
		msg.xclient.window = source;
		msg.xclient.data.l[0] = win;
		msg.xclient.data.l[1] = 0x1;
		msg.xclient.data.l[2] = atoms[XdndActionCopy];
		msg.xclient.data.l[3] = 0;
		msg.xclient.data.l[4] = 0;
		XSendEvent(dpy, source, False, NoEventMask, &msg);
		source = None;
	}
	else if (e->message_type == atoms[WmProtocols]) {
		if (e->data.l[0] == atoms[WmDeleteWindow]) {
			// IGNORE!
		}
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

void selectionnotify(XEvent *ev) {
	XSelectionEvent *e = &ev->xselection;
	if (e->property != atoms[FileName]) return;
	Atom type;
	int fmt;
	unsigned long n, nn;
	unsigned char *data;
	XGetWindowProperty(dpy, win, e->property, 0, 0x8000000L, False,
			AnyPropertyType, &type, &fmt, &n, &nn, &data);
	image_load(data);
	XFree(data);
}

