/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

int config_free() {
	return 0;
}

int config_init(int argc, const char **argv) {
	/* start with reasonable defaults: */
	conf.threshold.low = (Col) {0, 0, 0, 255};
	conf.threshold.hi = (Col) {80, 80, 80, 255};
	conf.threshold.pseudo = (Col) {0, 0, 0, 0};
	conf.alpha = 64;
	conf.line = (Col) {255, 180, 40, 255};
	conf.prompt = NULL;
	imgs = NULL;
	/* loop through args: */
	const char *arg, *prev, **argp;
	Col c1, c2;
	int ret;
	for (argp = (++argv), arg = *argp; arg && argp; arg = *(++argp)) {
		if (arg[0] == '-') {
			/* simple flags: */
			// TODO -h --help -v --version
			/* options with required arguments: */
			prev = arg;
			arg = *(++argp);
			if (!arg || arg[0] == '-') {
				fprintf(stderr, "%s missing required argument\n", prev);
				continue;
			}
			if (prev[0] == '-') prev++;
			switch (prev[0]) {
				case 'a': /* -a --alpha */
					ret = sscanf(arg,"%d", &c1.a);
					if (!ret) fprintf(stderr, "bad argument for %s\n", prev);
					else conf.alpha = c1.a;
					break;
				case 'c': /* -c --color */
					ret = sscanf(arg,"%d,%d,%d,%d", &c1.r, &c1.g, &c1.b, &c1.a);
					if (ret < 3) fprintf(stderr, "bad argument for %s\n", prev);
					else conf.threshold.pseudo = c1;
					if (ret == 3) conf.threshold.pseudo.a = 255;
					break;
				case 'l': /* -l --line */
					ret = sscanf(arg,"%d,%d,%d,%d", &c1.r, &c1.g, &c1.b, &c1.a);
					if (ret < 3) fprintf(stderr, "bad argument for %s\n", prev);
					else conf.line = c1;
					if (ret == 3) conf.line.a = 255;
					break;
				case 'p': /* -p --prompt */
					/* IGNORE: prompt is processed by console process */
					break;
				case 't': /* -t --threshold */
					ret = sscanf(arg,"%d,%d,%d,%d,%d,%d",
							&c1.r, &c1.g, &c1.b, &c2.r, &c2.g, &c2.b);
					if (ret != 6) fprintf(stderr, "bad argument for %s\n", prev);
					else {
						conf.threshold.low = c1;
						conf.threshold.hi = c2;
					}
					break;
				default:
					fprintf(stderr,"unrecognized parameter \"%s\" ignored.\n",
							(arg = *(--argp)));
			}
		}
		else cairo_create_img(arg);
	}
	return 0;
}

