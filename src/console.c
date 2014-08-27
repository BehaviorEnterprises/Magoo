/**********************************************************************\
* MAGOO - Image analysis software
* Author: Jesse McClure, copyright 2012-2014
* License: GPLv3
* See COPYING for details
\**********************************************************************/

#include "magoo.h"

static int process_init();
static int console_loop();
static int help_version(char);

static const char *_prompt = "\n\033[34;1;4mMa\033[0;34;1mg\033[4moo\033[0;34;1m:\033[0m ";
static const char *prompt;
static Bool ready_for_input = False;
static Bool shutting_down = False;


char *command_completion(const char *text, int state) {
	static int i, len;
	if (!state) { i = -1; len = strlen(text); }
	for (i++; commands[i].name; i++)
		if (!strncasecmp(commands[i].name, text, len))
			return strdup(commands[i].name);
	return NULL;
}

char **magoo_completion(const char *text, int start, int end) {
	if (start) {
		if (strncasecmp(rl_line_buffer,"open", 4) == 0)
			return NULL;
		if (strncasecmp(rl_line_buffer,"sink", 4) == 0)
			return NULL;
		if (strncasecmp(rl_line_buffer,"shell", 5) == 0)
			return NULL;
		rl_attempted_completion_over = 1;
		if (strncasecmp(rl_line_buffer,"help", 4) == 0)
			return rl_completion_matches(text, command_completion);
		return NULL;
	}
	return rl_completion_matches(text, command_completion);
}

void sig_handler(int sig) {
	if (sig == SIGUSR1) ready_for_input = True;
	if (sig == SIGUSR2) shutting_down = True;
}

int process_init() {
	signal(SIGUSR1, &sig_handler);
	signal(SIGUSR2, &sig_handler);
	rl_attempted_completion_function = magoo_completion;
}

int console_loop(int wfd) {
	char *cmd = NULL;
	int status;
	fprintf(stdout,"\033]0;Magoo: Console\007");
	system("clear");
	for (;;) {
		while (!ready_for_input) sleep(1);
		if (shutting_down) return 0;
		cmd = readline(prompt);
		if (!cmd) cmd = strdup("exit");
		if (!*cmd || strlen(cmd) > 255) {
			free(cmd);
			continue;
		}
		add_history(cmd);
		ready_for_input = False;
		write(wfd, cmd, strlen(cmd)+1);
		free(cmd);
	}
	free(cmd);
	return 0;
}

int console_init(int argc, const char **argv) {
	/* check for prompt on command line: */
	prompt = _prompt;
	int i;
	const char *c;
	for (i = 0; i < argc; i++) {
		if ((c = argv[i])[0] != '-') continue;
		if ( *(++c) == '-') ++c;
		switch (*c) {
			case 'p': prompt = argv[i+1]; //break;
			case 'h': case 'v': help_version(*c); break;
			case 'a': case 'c': case 'l': case 't': break;
			default: break;//TODO error
		}
	}
	/* fork a new process for console input: */
	int fd[2];
	pipe(fd);
	if ( (terminal=fork()) == 0) {
		close(fd[0]);
		process_init();
		console_loop(fd[1]);
		exit(0);
	}
	/* return to gui process: */
	close(fd[1]);
	input = fd[0];
	return 0;
}

int help_version(char c) {
	printf(VERSION_STRING);
	if (c == 'v') exit(0);
	printf("\nSee `man magoo` for usage details or use the `help` command in an interactive session\n");
	exit(0);
	return 0;
}

