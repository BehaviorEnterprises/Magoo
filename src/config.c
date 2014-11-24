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
	uint8_t n, step = 255 / NTHRESH, res, rem;
	Col last = (Col) {0, 0, 0, 255};
	for (n = 0; n < NTHRESH; ++n) {
		conf.thresh[n].low = last;
		last.r += step; last.g += step; last.b += step;
		conf.thresh[n].hi = last;
		res = (n / 6) + 1; rem = n % 6;
		conf.thresh[n].pseudo = (Col) { /* yellow, blue, red, green, purple, black */
			(int) (rem % 2 == 0 ? 255 : 0) / res,
			(int) (rem==0 ? 255 : (rem==3 ? 255 : 0)) / res,
			(int) (rem==1 ? 255 : (rem==4 ? 255 : 0)) / res,
			255,
		};
	}
	conf.thresh[NTHRESH - 1].hi = (Col) { 255, 255, 255, 255 };
	conf.alpha = 255;
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
				case 'c': /* -c --color */ // TODO
					ret = sscanf(arg,"%d,%d,%d,%d", &c1.r, &c1.g, &c1.b, &c1.a);
					if (ret < 3) fprintf(stderr, "bad argument for %s\n", prev);
					else conf.thresh[0].pseudo = c1;
					if (ret == 3) conf.thresh[0].pseudo.a = 255;
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
				case 't': /* -t --threshold */ // TODO
					ret = sscanf(arg,"%d,%d,%d,%d,%d,%d",
							&c1.r, &c1.g, &c1.b, &c2.r, &c2.g, &c2.b);
					if (ret != 6) fprintf(stderr, "bad argument for %s\n", prev);
					else {
						conf.thresh[0].low = c1;
						conf.thresh[0].hi = c2;
					}
					break;
				default:
					fprintf(stderr,"unrecognized parameter \"%s\" ignored.\n",
							(arg = *(--argp)));
			}
		}
		else image_load(arg);
	}
	return 0;
}

