// BE HEX DASM BINARY EDITOR 2022 Groupoid Infinity

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "editor.h"
#include "terminal.h"
#include "dasm/dasm.h"

static struct editor* e;
volatile sig_atomic_t resizeflag;

static void editor_exit() {
    editor_free(e);
    clear_screen();
    disable_raw_mode();
    term_state_restore();
}

static void print_help(const char* explanation) {
    fprintf(stderr,
        "%s"\
        "usage: BE [-vhdbo] filename\n"\
        "\n"
        "Options:\n"
        "    -v           Get version information\n"
        "    -h           Print usage info and exits\n"
        "    -d           Launch ASM view by default\n"
        "    -b bit       CPU Bitness\n"
        "    -o octets    Octets per screen for HEX view\n"
        "\n"
        "Report bugs to <namdak@tonpa.guru>\n", explanation);
}

void print_version() {
    printf("BE version %s codename %s released %s\n", XT_VERSION, XT_RELEASE_CODENAME, XT_RELEASE_DATE);
}

static void handle_term_resize(int sig) {
    (void)(sig);
    resizeflag = 1;
}

static void resize_term() {
    clear_screen();
    get_window_size(&(e->screen_rows), &(e->screen_cols));
}

int main(int argc, char* argv[]) {
    char* file = NULL;
    int ch = 0, bitness = 64, opl = 24, view = 0;
    while ((ch = getopt(argc, argv, "vhdb:o:")) != -1) {
        switch (ch) {
            case 'v': print_version(); return 0;
            case 'h': print_help(""); exit(0); break;
            case 'o': opl = str2int(optarg, 16, 64, 16); break;
            case 'b': bitness = str2int(optarg, 16, 64, 16); break;
            case 'd': view = VIEW_ASM; break;
            default: print_help(""); exit(1); break;
        }
    }

    if (optind >= argc) {
        print_help("Error: filename is expected.\n");
        exit(1);
    }

    file = argv[optind];

    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = handle_term_resize;
    sigaction(SIGWINCH, &act, NULL);
    resizeflag = 0;

    e = editor_init();
    editor_openfile(e, file);
    enable_raw_mode();
    term_state_save();
    atexit(editor_exit);
    clear_screen();
    e->octets_per_line = opl;
    e->seg_size = bitness;
    editor_setview(e, view ? VIEW_ASM : VIEW_HEX);
    nasm_init(e);

    while (true) {
        editor_refresh_screen(e);
        editor_process_keypress(e);
        if (resizeflag == 1) {
            resize_term();
            resizeflag = 0;
        }
    }

    editor_free(e);
    return 0;
}
