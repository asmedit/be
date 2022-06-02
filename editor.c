#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dasm/dasm.h"
#include "hex/hex.h"
#include "editor.h"
#include "terminal.h"

void editor_newfile(struct editor* e, const char* filename) {
	e->filename = malloc(strlen(filename) + 1);
	strncpy(e->filename, filename, strlen(filename) + 1);
	e->contents = malloc(0);
	e->content_length = 0;
}

void editor_openfile(struct editor* e, const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		if (errno == ENOENT) {
			editor_newfile(e, filename);
			return;
		}
		perror("Unable to open file");
		exit(1);
	}

	struct stat statbuf;
	if (stat(filename, &statbuf) == -1) {
		perror("Cannot stat file");
		exit(1);
	}
	if (!S_ISREG(statbuf.st_mode)) {
		fprintf(stderr, "File '%s' is not a regular file\n", filename);
		exit(1);
	}

	char* contents;
	int content_length = 0;

	if (statbuf.st_size <= 0) {
		struct charbuf* buf = charbuf_create();
		int c;
		char tempbuf[1];
		while ((c = fgetc(fp)) != EOF) {
			tempbuf[0] = (char) c;
			charbuf_append(buf, tempbuf, 1);
		}
		contents = buf->contents;
		content_length = buf->len;
	} else {
		contents = malloc(sizeof(char) * statbuf.st_size);
		content_length = statbuf.st_size;

		if (fread(contents, 1, statbuf.st_size, fp) < (size_t) statbuf.st_size) {
			perror("Unable to read file contents");
			free(contents);
			exit(1);
		}
	}

	e->filename = malloc(strlen(filename) + 1);
	strncpy(e->filename, filename, strlen(filename) + 1);
	e->contents = contents;
	e->content_length = content_length;

	if (access(filename, W_OK) == -1) {
		editor_statusmessage(e, STATUS_WARNING, "\"%s\" (%d bytes) [readonly]", e->filename, e->content_length);
	} else {
		editor_statusmessage(e, STATUS_INFO, "\"%s\" (%d bytes)", e->filename, e->content_length);
	}

	fclose(fp);
}

void editor_writefile(struct editor* e) {
	assert(e->filename != NULL);

	FILE* fp = fopen(e->filename, "wb");
	if (fp == NULL) {
		editor_statusmessage(e, STATUS_ERROR, "Unable to open '%s' for writing: %s", e->filename, strerror(errno));
		return;
	}

	size_t bw = fwrite(e->contents, sizeof(char), e->content_length, fp);
	if (bw <= 0) {
		editor_statusmessage(e, STATUS_ERROR, "Unable write to file: %s", strerror(errno));
		return;
	}

	editor_statusmessage(e, STATUS_INFO, "\"%s\", %d bytes written", e->filename, e->content_length);
	e->dirty = false;

	fclose(fp);
}

void editor_setmode(struct editor* e, enum editor_mode mode) {
	e->mode = mode;
	switch (e->mode) {
	case MODE_NORMAL:        editor_statusmessage(e, STATUS_INFO, ""); break;
	case MODE_APPEND:        editor_statusmessage(e, STATUS_INFO, "-- APPEND -- "); break;
	case MODE_APPEND_ASCII:  editor_statusmessage(e, STATUS_INFO, "-- APPEND ASCII --"); break;
	case MODE_REPLACE_ASCII: editor_statusmessage(e, STATUS_INFO, "-- REPLACE ASCII --"); break;
	case MODE_INSERT:        editor_statusmessage(e, STATUS_INFO, "-- INSERT --"); break;
	case MODE_INSERT_ASCII:  editor_statusmessage(e, STATUS_INFO, "-- INSERT ASCII --"); break;
	case MODE_REPLACE:       editor_statusmessage(e, STATUS_INFO, "-- REPLACE --"); break;
	case MODE_COMMAND: break;
	case MODE_SEARCH:  break;
	case MODE_DASM:          editor_statusmessage(e, STATUS_INFO, "-- DASM --"); break;
	}
}

int editor_statusmessage(struct editor* e, enum status_severity sev, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int x = vsnprintf(e->status_message, sizeof(e->status_message), fmt, ap);
	va_end(ap);
	e->status_severity = sev;
	return x;
}

void editor_render_header(struct editor* e, struct charbuf* b) {
	char banner[ 1024 + 1];
	int  banlen = 0;
    int current_offset =  editor_offset_at_cursor(e);
	unsigned char active_byte = e->contents[current_offset];
	banlen = snprintf(banner, sizeof(banner), "\x1b[1;1;42m XT 8086 [HEX][%02x][%08x] ", active_byte, current_offset);
	charbuf_append(b, banner, banlen);

	unsigned int offset_at_cursor = editor_offset_at_cursor(e);
	unsigned char val = e->contents[offset_at_cursor];
	int percentage = (float)(offset_at_cursor + 1) / ((float)e->content_length) * 100;
	int file_position = snprintf(banner, sizeof(banner), "%28c% 15d%% ", ' ', percentage);
	charbuf_append(b, banner, file_position);
	charbuf_append(b, "\r\n", 2);
	charbuf_append(b, "\x1b[0m\x1b[K", 7);
}

void editor_render_contents(struct editor* e, struct charbuf* b) {
    switch (e->mode) {
        case MODE_DASM: editor_render_dasm(e, b); break;
        default:        editor_render_hex(e, b);
    }
}

void editor_render_help(struct editor* e) {
	(void) e;
	struct charbuf* b = charbuf_create();
	clear_screen();
	charbuf_append(b, "\x1b[?25l", 6); // hide cursor
	charbuf_appendf(b, "This is xt, version %s\r\n\n", XT_VERSION);
	charbuf_appendf(b,
		"Available commands:\r\n"
		"\r\n"
		"CTRL+Q  : Quit immediately without saving.\r\n"
		"CTRL+S  : Save (in place).\r\n"
		"Arrows  : Also moves the cursor around.\r\n"
		"\r\n");
	charbuf_appendf(b,
		":       : Command mode. Commands can be typed and executed.\r\n"
		"ESC     : Return to normal mode.\r\n"
		"End     : Move cursor to end of the offset line.\r\n"
		"Home    : Move cursor to the beginning of the offset line.\r\n"
		"\r\n"
	);
	charbuf_appendf(b,
		"Press any key to exit help.\r\n");

	charbuf_draw(b);

	read_key();
	clear_screen();
}

void editor_render_status(struct editor* e, struct charbuf* b) {
	charbuf_appendf(b, "\x1b[%d;0H", e->screen_rows);
	switch (e->status_severity) {
      case STATUS_INFO:    charbuf_append(b, "\x1b[0;30;47m", 10); break; // black on white
	  case STATUS_WARNING: charbuf_append(b, "\x1b[0;30;43m", 10); break; // black on yellow
	  case STATUS_ERROR:   charbuf_append(b, "\x1b[1;37;41m", 10); break; // white on red
	}
	int maxchars = strlen(e->status_message);
	if (e->screen_cols <= maxchars) {
		maxchars = e->screen_cols;
	}
	charbuf_append(b, e->status_message, maxchars);
	charbuf_append(b, "\x1b[0m\x1b[0K", 8);
}


void editor_refresh_screen(struct editor* e) {
	struct charbuf* b = charbuf_create();

	charbuf_append(b, "\x1b[?25l", 6);
	charbuf_append(b, "\x1b[H", 3); // move the cursor top left

	if (e->mode &
			(MODE_REPLACE |
			 MODE_NORMAL |
			 MODE_APPEND |
			 MODE_APPEND_ASCII |
			 MODE_REPLACE_ASCII |
			 MODE_INSERT |
			 MODE_DASM |
			 MODE_INSERT_ASCII)) {

		editor_render_header(e, b);
		editor_render_contents(e, b);
		editor_render_status(e, b);

	} else if (e->mode & MODE_COMMAND) {

		charbuf_appendf(b, "\x1b[0m\x1b[?25h\x1b[%d;1H\x1b[2K:", e->screen_rows);
		charbuf_append(b, e->inputbuffer, e->inputbuffer_index);

	}

	charbuf_draw(b);
	charbuf_free(b);
}

void editor_process_command(struct editor* e, const char* cmd) {
	bool b = is_pos_num(cmd);
	if (b) {
		int offset = str2int(cmd, 0, e->content_length, e->content_length - 1);
		editor_scroll_to_offset(e, offset);
		editor_statusmessage(e, STATUS_INFO, "Positioned to offset 0x%09x (%d)", offset, offset);
		return;
	}

	if (cmd[0] == '0' && cmd[1] == 'x') {
		const char* ptr = &cmd[2];
		if (!is_hex(ptr)) {
			editor_statusmessage(e, STATUS_ERROR, "Error: %s is not valid base 16", ptr);
			return;
		}

		int offset = hex2int(ptr);
		editor_scroll_to_offset(e, offset);
		editor_statusmessage(e, STATUS_INFO, "Positioned to offset 0x%09x (%d)", offset, offset);
		return;
	}

	if (strncmp(cmd, "w", INPUT_BUF_SIZE) == 0) {
		editor_writefile(e);
		return;
	}

	if (strncmp(cmd, "q", INPUT_BUF_SIZE) == 0) {
		if (e->dirty) {
			editor_statusmessage(e, STATUS_ERROR, "No write since last change (add ! to override)", cmd);
			return;
		} else {
			exit(0);
		}
	}

	if (strncmp(cmd, "q!", INPUT_BUF_SIZE) == 0) {
		exit(0);
		return;
	}

	if (strncmp(cmd, "help", INPUT_BUF_SIZE) == 0) {
		editor_render_help(e);
		return;
	}

	if (strncmp(cmd, "set", 3) == 0) {
		char setcmd[INPUT_BUF_SIZE] = {0};
		int setval = 0;
		int items_read = sscanf(cmd, "set %[a-z]=%d", setcmd, &setval);

		if (items_read != 2) {
			editor_statusmessage(e, STATUS_ERROR, "set command format: `set cmd=num`");
			return;
		}

		if (strcmp(setcmd, "octets") == 0 || strcmp(setcmd, "o") == 0) {
			int octets = clampi(setval, 16, 64);

			clear_screen();
			int offset = editor_offset_at_cursor(e);
			e->octets_per_line = octets;
			editor_scroll_to_offset(e, offset);

			editor_statusmessage(e, STATUS_INFO, "Octets per line set to %d", octets);

			return;
		}

		if (strcmp(setcmd, "grouping") == 0 || strcmp(setcmd, "g") == 0) {
			int grouping = clampi(setval, 4, 16);
			clear_screen();
			e->grouping = grouping;

			editor_statusmessage(e, STATUS_INFO, "Byte grouping set to %d", grouping);
			return;
		}

		editor_statusmessage(e, STATUS_ERROR, "Unknown option: %s", setcmd);
		return;
	}

	editor_statusmessage(e, STATUS_ERROR, "Command not found: %s", cmd);
}


int editor_read_string(struct editor* e, char* dst, int len) {
	int c = read_key();
	if (c == KEY_ENTER || c == KEY_ESC) {
		editor_setmode(e, MODE_NORMAL);
		strncpy(dst, e->inputbuffer, len);
		e->inputbuffer_index = 0;
		memset(e->inputbuffer,  '\0', sizeof(e->inputbuffer));
		return c;
	}

	if (c == KEY_BACKSPACE && e->inputbuffer_index > 0) {
		e->inputbuffer_index--;
		e->inputbuffer[e->inputbuffer_index] = '\0';
		return c;
	}

	if ((size_t) e->inputbuffer_index >= sizeof(e->inputbuffer) - 1) {
		return c;
	}

	if (c == KEY_BACKSPACE && e->inputbuffer_index == 0) {
		editor_setmode(e, MODE_NORMAL);
		return c;
	}

	if (!isprint(c)) {
		return c;
	}

	e->inputbuffer[e->inputbuffer_index++] = c;
	return c;
}


void editor_process_keypress(struct editor* e) {
    switch (e->mode) {
        case MODE_DASM: editor_process_keypress_dasm(e); break;
        default:        editor_process_keypress_hex(e);
    }
}

struct editor* editor_init() {
	struct editor* e = malloc(sizeof(struct editor));
	e->octets_per_line = 16;
	e->grouping = 2;
	e->line = 0;
	e->cursor_x = 1;
	e->cursor_y = 1;
	e->filename = NULL;
	e->contents = NULL;
	e->content_length = 0;
	e->dirty = false;
	memset(e->status_message, '\0', sizeof(e->status_message));
	e->mode = MODE_NORMAL;
	memset(e->inputbuffer, '\0', sizeof(e->inputbuffer));
	e->inputbuffer_index = 0;
	memset(e->searchstr, '\0', sizeof(e->searchstr));
	get_window_size(&(e->screen_rows), &(e->screen_cols));
	return e;
}

void editor_free(struct editor* e) {
	free(e->filename);
	free(e->contents);
	free(e);
}
