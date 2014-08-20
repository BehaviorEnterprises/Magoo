
#include "magoo.h"

static FILE *log_file;
static Bool ready_for_input = False;

void die(const char *fmt, ...) {
	fprintf(stderr,"[SCOPE] ");
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
	fprintf(stderr,"\n");
	exit(1);
}

void log_entry(const char *fmt, ...) {
	fprintf(log_file,"[SCOPE] ");
	va_list arg;
	va_start(arg, fmt);
	vfprintf(log_file, fmt, arg);
	va_end(arg);
	fprintf(log_file,"\n");
}

int main(int argc, const char **argv) {
	command_init();
	console_init(argc, argv);
	log_file = stderr;
	imgs = NULL;
	config_init(argc, argv);
	xlib_init();
	focused_img = imgs;
	main_loop();
	xlib_free();
	config_free();
	return 0;
}
