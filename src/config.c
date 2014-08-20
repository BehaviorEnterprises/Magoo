
#include "magoo.h"

int config_free() {
	return 0;
}

int config_init(int argc, const char **argv) {
	imgs = NULL;
	const char *arg, **argp;
	for (argp = (++argv), arg = *argp; arg && argp; arg = *(++argp)) {
		cairo_create_img(arg);
	}
	return 0;
}

