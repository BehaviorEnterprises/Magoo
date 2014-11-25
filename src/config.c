/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

int config_free() {
	free(conf.thresh);
	if (conf.prompt) free(conf.prompt);
	return 0;
}

int config_init(int argc, const char **argv) {
	conf.prompt = NULL;
	conf.layers = True;
	imgs = NULL;
	FILE *in = NULL;
	uint8_t n;
	uint32_t u1, u2;
	char line[256], var[64], *env;
	conf.draw.a = MODE_POLY;
	for (n = 1; n < argc - 1; ++n)
		if (argv[n][0] == '-' && argv[n][1] == 'F')
			in = fopen(argv[n+1], "r");
	if (!in)
		in = fopen(".magoorc", "r");
	if (!in && (env=getenv("XDG_CONFIG_HOME"))) {
		chdir(env);
		in = fopen("magoo/config", "r");
	}
	if (!in && (env=getenv("HOME"))) {
		chdir(env);
		in = fopen(".config/magoo/config", "r");
	}
	if (!in)
		in = fopen("/etc/xdg/magoo/config", "r");
	if (!in) exit(1); // TODO
	while (fgets(line, 256, in))
		if (sscanf(line, "levels %hhu\n", &n) == 1) conf.levels = n;
	conf.thresh = calloc(n, sizeof(Threshold));
	fseek(in, 0L, SEEK_SET);
	while (fgets(line, 256, in)) {
		if (line[0] == '#')
			continue;
		else if (sscanf(line, "levels %hhu\n", &n) == 1)
			continue;
		else if (sscanf(line, "alpha %hhu", &n) == 1)
			conf.alpha = n;
		else if (sscanf(line, "line %X", &u1) == 1)
			conf.line.u = u1;
		else if (sscanf(line, "range%hhu %X %X", &n, &u1, &u2) == 3) {
			if (n && n <= conf.levels) {
				conf.thresh[n-1].low.u = u1;
				conf.thresh[n-1].hi.u = u2;
			}
		}
		else if (sscanf(line, "color%hhu %X", &n, &u1) == 2) {
			if (n && n <= conf.levels) {
				conf.thresh[n-1].pseudo.u = u1;
			}
		}
		else
			fprintf(stderr, "unrecognized configuration entry \"%s\"\n", line);
	}
	for (n = 1; n < argc; ++n)
		if (argv[n][0] != '-') image_load(argv[n]);
	return 0;
}

