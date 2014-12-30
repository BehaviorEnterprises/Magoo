
#include "magoo.h"

int note_create_label(Img *img, int x, int y) {
	/* create new note */
	img->note = realloc(img->note, (img->nnotes + 1) * sizeof(Note));
	/* set coordinates and initialize entries */
	img->note[img->nnotes].x = x;
	img->note[img->nnotes].y = y;
	memset(img->note[img->nnotes].entry, 0, MAX_COLUMNS * sizeof(char *));
	/* set curnote to the new note */
	++img->nnotes;
	img->curnote = img->nnotes;
	return 0;
}

int note_set_current(Img *img, int n) {
	if (n > img->nnotes) return 1;
	img->curnote = n;
}

int note_entry(Img *img, int col, const char *data) {
	if (!img->curnote) return 1;
	Note *n = &img->note[img->curnote - 1];
	int i;
	/* free any existing contents of selected column */
	if (n->entry[col]) free(n->entry[col]);
	/* dup data into selected column */
	n->entry[col] = strdup(data);
	return 0;
}

int free_notes(Img *img) {
	int i, j;
	Note *n = NULL;
	for (i = 0; i < img->nnotes; ++i) {
		n = &img->note[i];
		for (j = 0; j < MAX_COLUMNS; ++j) {
			if (n->entry[j]) free(n->entry[j]);
		}
	}
	return 0;
}

#define LINE_LENGTH	255
int note_read_file(Img *img) {
	if (!img) return 1;
	/* open file */
	FILE *in;
	in = fopen(img->notes_file, "r");
	if (!in) return 1;
	char line[LINE_LENGTH], entries[LINE_LENGTH];
	int n, x, y;
	fgets(line, LINE_LENGTH, in);
	/* read header, ensure it's correct */
	if (strncmp(line, "Label, X, Y", 11) != 0) return 2;
	int ncol = 0;
	for (n = 0; cmd_names[n]; ++n)
		if (strstr(line, cmd_names[n])) img->columns[ncol++] = n + 1;
	img->columns[ncol] = 0;
	/* loop through lines of file, reading x, y, and all entries */
	while (fgets(line, LINE_LENGTH, in)) {
		entries[0] = '\0';
		if (sscanf(line, "%d, %d, %d, %[^\n]\n", &n, &x, &y, entries) < 3)
			continue;
		/* create a new label */
		note_create_label(img, x, y);
		char *tok, *sptr, *str, *tokptr;
		int i = 0;
		if (entries[0] == ',') ++i;
		/* tokenize entries */
		str = strdup(entries);
		for (tok=strtok_r(str,",",&sptr); tok; tok=strtok_r(NULL,",",&sptr)) {
			/* clean leading spaces and dup token to entry */
			for (tokptr = tok; *tokptr == ' '; ++tokptr);
			note_entry(img, i++, tokptr);
			tokptr = NULL;
		}
		/* free the tokenization temporary string */
		free(str);
	}
	img->curnote = 0;
	return 0;
}

int note_write_file(Img *img) {
	/* open file, create header */
	FILE *out = NULL;
	if (img) {
		/* skip file creation if there are no notes to write */
		if (!img->nnotes) return 0;
		/* open file */
		out = fopen(img->notes_file, "w");
		if (!out) {
			fprintf(stderr,"error opening\n"); // TODO
			return 1;
		}
	}
	else {
		out = stdout;
		img = focused_img;
	}
	Note *n;
	int i, j;
	fprintf(out, "Label, X, Y");
	for (i = 0; img->columns[i]; ++i)
		fprintf(out, ", %s", cmd_names[img->columns[i] - 1]);
	fprintf(out, "\n");
	/* loop through notes, print coordinates */
	for (i = 0; i < img->nnotes; i++) {
		n = &img->note[i];
		fprintf(out, "%d, %d, %d", i + 1, n->x, n->y);
		/* loop through entries, print entries */
		for (j = 0; img->columns[j]; ++j) {
			/* print entry, or blank column in the absence of an entry */
			if (n->entry[j]) fprintf(out, ", %s", n->entry[j]);
			else fprintf(out, ", ");
		}
		/* finish the line */
		fprintf(out, "\n");
	}
	if (out != stdout) fclose(out);
}

