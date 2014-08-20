
#include "magoo.h"

static int process_init();
static int console_loop();

static const char *prompt = "\n\033[34;1;4mMa\033[0;34;1mg\033[4moo\033[0;34;1m:\033[0m ";
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

char **scope_completion(const char *text, int start, int end) {
	if (start) return NULL;
	return rl_completion_matches(text, command_completion);
}

void sig_handler(int sig) {
	if (sig == SIGUSR1) ready_for_input = True;
	if (sig == SIGUSR2) shutting_down = True;
}

int process_init() {
	signal(SIGUSR1, &sig_handler);
	signal(SIGUSR2, &sig_handler);
	rl_attempted_completion_function = scope_completion;
}

int console_loop(int wfd) {
	char *cmd = NULL;
	int status;
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
//		if (strncasecmp(cmd,"exit",4) == 0) break;
//		if (strncasecmp(cmd,"quit",4) == 0) break;
		free(cmd);
	}
	free(cmd);
	return 0;
}

int console_init(int argc, const char **argv) {
	int fd[2];
	pipe(fd);
	if ( (terminal=fork()) == 0) {
		close(fd[0]);
		process_init();
		console_loop(fd[1]);
		exit(0);
	}
	close(fd[1]);
	input = fd[0];
	return 0;
}

