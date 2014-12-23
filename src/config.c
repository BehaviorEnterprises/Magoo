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
	uint32_t u1, u2, u3, u4;
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
		if (line[0] == '#' || line[0] == '\n')
			continue;
		else if (sscanf(line, "levels %hhu\n", &n) == 1)
			continue;
		else if (sscanf(line, "alpha %hhu", &n) == 1)
			conf.alpha = n;
		else if (sscanf(line, "line %X", &u1) == 1)
			conf.line.u = u1;
		else if (sscanf(line, "label %X %X %X %X", &u1, &u2, &u3, &u4) == 4) {
			conf.fgNote.u = u1; conf.bgNote.u = u2;
			conf.fgCurNote.u = u3; conf.bgCurNote.u = u4;
		}
		else if (strncmp(line, "column", 6) == 0) {
			u2 = 0;
			for (u1 = 0; cmd_names[u1]; ++u1)
				if (strstr(line, cmd_names[u1])) conf.columns[u2++] = u1 + 1;
		}
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
		else {
			line[strlen(line)-1] = '\0';
			fprintf(stderr, "unrecognized configuration entry \"%s\"\n", line);
		}
	}
	for (n = 1; n < argc; ++n)
		if (argv[n][0] != '-') image_load(argv[n]);
	return 0;
}

