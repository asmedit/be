#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "editor.h"
#include "terminal.h"

static struct editor* g_ec;
volatile sig_atomic_t resizeflag;

static void editor_exit() {
 	editor_free(g_ec);
 	clear_screen();
 	disable_raw_mode();
	term_state_restore();
}

static void print_help(const char* explanation) {
	fprintf(stderr,
	"%s"\
	"usage: xt [-hvm] filename\n"\
	"\n"
	"Command options:\n"
	"    -h     Print usage info and exits\n"
	"    -v     Get version information\n"
	"    -m     Mode ASM/HEX/TEXT\n"
	"\n"
	"Report bugs to <maxim@synrc.com>\n"
	, explanation);
}

static void print_version() {
	printf("xt version %s (git: %s)\n", XT_VERSION, XT_GIT_HASH);
}

static void handle_term_resize(int sig) {
	(void)(sig);
	resizeflag = 1;
}

static void resize_term() {
	clear_screen();
	get_window_size(&(g_ec->screen_rows), &(g_ec->screen_cols));
}

int main(int argc, char* argv[]) {
	char* file = NULL;
	int octets_per_line = 16;
	int grouping = 4;

	int ch = 0;
	while ((ch = getopt(argc, argv, "mvhg:o:")) != -1) {
		switch (ch) {
		case 'v':
			print_version();
			return 0;
		case 'h':
			print_help("");
			exit(0);
			break;
		case 'm':
			break;
		default:
			print_help("");
			exit(1);
			break;
		}
	}

	if (optind >= argc) {
		print_help("error: expected filename\n");
		exit(1);
	}

	file = argv[optind];

	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = handle_term_resize;
	sigaction(SIGWINCH, &act, NULL);
	resizeflag = 0;

	g_ec = editor_init();
	g_ec->octets_per_line = 16;
	g_ec->grouping = 1;

	editor_openfile(g_ec, file);
	enable_raw_mode();
	term_state_save();
	atexit(editor_exit);
	clear_screen();

	while (true) {
		editor_refresh_screen(g_ec);
		editor_process_keypress(g_ec);
		if (resizeflag == 1) {
			resize_term();
			resizeflag = 0;
		}
	}

	editor_free(g_ec);
	return 0;
}
