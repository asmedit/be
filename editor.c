#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

int hexstr_idx = 0;
char hexstr[2 + 1];

void editor_move_cursor(struct editor* e, int dir, int amount) {
	switch (dir) {
	case KEY_UP:    e->cursor_y-=amount; break;
	case KEY_DOWN:  e->cursor_y+=amount; break;
	case KEY_LEFT:  e->cursor_x-=amount; break;
	case KEY_RIGHT: e->cursor_x+=amount; break;
	}
	if (e->cursor_x <= 1 && e->cursor_y <= 1 && e->line <= 0) {
		e->cursor_x = 1;
		e->cursor_y = 1;
		return;
	}

	if (e->cursor_x < 1) {
		if (e->cursor_y >= 1) {
			e->cursor_y--;
			e->cursor_x = e->octets_per_line;
		}
	} else if (e->cursor_x > e->octets_per_line) {
		e->cursor_y++;
		e->cursor_x = 1;
	}

	if (e->cursor_y <= 1 && e->line <= 0) {
		e->cursor_y = 1;
	}

	if (e->cursor_y > e->screen_rows - 2) {
		e->cursor_y = e->screen_rows - 2;
		editor_scroll(e, 1);
	} else if (e->cursor_y < 1 && e->line > 0) {
		e->cursor_y = 1;
		editor_scroll(e, -1);
	}

	unsigned int offset = editor_offset_at_cursor(e);
	if (offset >= e->content_length - 1) {
		editor_cursor_at_offset(e, offset, &e->cursor_x, &e->cursor_y);
		return;
	}
}

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


void editor_cursor_at_offset(struct editor* e, int offset, int* x, int* y) {
	*x = offset % e->octets_per_line + 1;
	*y = offset / e->octets_per_line - e->line + 1;
}

inline int editor_offset_at_cursor(struct editor* e) {
	unsigned int offset = (e->cursor_y - 1 + e->line) * e->octets_per_line + (e->cursor_x - 1);
	if (offset <= 0) {
		return 0;
	}
	if (offset >= e->content_length) {
		return e->content_length - 1;
	}
	return offset;
}


void editor_scroll(struct editor* e, int units) {
	e->line += units;
	int upper_limit = e->content_length / e->octets_per_line - (e->screen_rows - 3);
	if (e->line >= upper_limit) e->line = upper_limit;
	if (e->line <= 0) e->line = 0;
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

void editor_render_ascii(struct editor* e, int rownum, unsigned int start_offset, struct charbuf* b) {
	int cc = 0;
	for (unsigned int offset = start_offset; offset < start_offset + e->octets_per_line; offset++) {
		if (offset >= e->content_length) return;
		cc++;
		char c =  e->contents[offset];
		if (rownum == e->cursor_y && cc == e->cursor_x) charbuf_append(b, "\x1b[1;3;43m", 10);
		else charbuf_appendf(b, "\x1b[1;3;44m", 9);
		if (isprint(c)) {
			charbuf_appendf(b, "%c", c);
		} else {
			charbuf_append(b, ".", 6);
		}
	}
	charbuf_append(b, "\x1b[0m\x1b[K", 7);
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

	if (e->content_length <= 0) {
		charbuf_append(b, "\x1b[2J", 4);
		charbuf_appendf(b, "File is empty. Use 'i' to insert a hexadecimal value.");
		return;
	}

	char hex[ 32 + 1];
	int  hexlen = 0;
	char asc[256 + 1];
	int row_char_count = 0;

	unsigned int start_offset = e->line * e->octets_per_line;
	if (start_offset >= e->content_length) {
		start_offset = e->content_length - e->octets_per_line;
	}

	int bytes_per_screen = (e->screen_rows - 1) * e->octets_per_line;
	unsigned int end_offset = bytes_per_screen + start_offset - e->octets_per_line;
	if (end_offset > e->content_length) {
		end_offset = e->content_length;
	}

	unsigned int offset;
	int row = 0;
	int col = 0;
    int current_offset =  editor_offset_at_cursor(e);

	for (offset = start_offset; offset < end_offset; offset++) {
		unsigned char curr_byte = e->contents[offset];

		if (offset % e->octets_per_line == 0) {
			charbuf_appendf(b, "\x1b[12;3;45m%09x\x1b[0m", offset);
			memset(asc, '\0', sizeof(asc));
			row_char_count = 0;
			col = 0;
			row++;
		}
		col++;

        hexlen = snprintf(hex, sizeof(hex), "%02x", curr_byte);

		if (isprint(curr_byte)) {
			asc[offset % e->octets_per_line] = curr_byte;
		} else {
			asc[offset % e->octets_per_line] = '.';
		}

		if (offset % e->grouping == 0) {
		    if (row_char_count % (e->octets_per_line / 4) != 0)
		         charbuf_append(b, "\x1b[12;3;44m ", 11);
		    else charbuf_append(b, "\x1b[12;3;47m ", 11);
			row_char_count++;
		}


		if (e->cursor_y == row && e->cursor_x == col) {
		     charbuf_append(b, "\x1b[1;4;43m ", 9);
             memset(&hexstr[1], '\0', 1);
             if (hexstr_idx == 1 && e->mode == MODE_REPLACE)
                 hexlen = snprintf(hex, sizeof(hex), "%02x",
                     curr_byte & 0xF | hex2bin(hexstr) & 0xF << 4);
		} else {
		     charbuf_append(b, "\x1b[3;3;44m ", 9);
		}

        charbuf_append(b, hex, hexlen);
	    charbuf_append(b, "\x1b[0m", 4);

		row_char_count += 2;

		if ((offset+1) % e->octets_per_line == 0) {
			charbuf_append(b, "\x1b[1;6;47m ", 10);
			int the_offset = offset + 1 - e->octets_per_line;
			editor_render_ascii(e, row, the_offset, b);
			charbuf_append(b, "\r\n", 2);
		}
	}

	charbuf_append(b, "\x1b[0K", 4);

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

void editor_scroll_to_offset(struct editor* e, unsigned int offset) {
	if (offset > e->content_length) {
		editor_statusmessage(e, STATUS_ERROR, "Out of range: 0x%09x (%u)", offset, offset);
		return;
	}

	unsigned int offset_min = e->line * e->octets_per_line;
	unsigned int offset_max = offset_min + (e->screen_rows * e->octets_per_line);

	if (offset >= offset_min && offset <= offset_max) {
		editor_cursor_at_offset(e, offset, &(e->cursor_x), &(e->cursor_y));
		return;
	}

	e->line = offset / e->octets_per_line - (e->screen_rows / 2);

	int upper_limit = e->content_length / e->octets_per_line - (e->screen_rows - 2);
	if (e->line >= upper_limit) {
		e->line = upper_limit;
	}

	if (e->line <= 0) {
		e->line = 0;
	}

	editor_cursor_at_offset(e, offset, &(e->cursor_x), &(e->cursor_y));
}


int editor_read_hex_input(struct editor* e, char* out) {

	int next = read_key();

	if (next == KEY_ESC) {
		editor_setmode(e, MODE_NORMAL);
		memset(hexstr, '\0', 3);
		hexstr_idx = 0;
		return -1;
	}

	if (!isprint(next)) {
//		editor_statusmessage(e, STATUS_ERROR, "Error: unprintable character (%02x)", next);
		return -1;
	}
	if (!isxdigit(next)) {
//		editor_statusmessage(e, STATUS_ERROR, "Error: '%c' (%02x) is not valid hex", next, next);
		return -1;
	}

	hexstr[hexstr_idx++] = next;

	if (hexstr_idx >= 2) {
		*out = hex2bin(hexstr);
		memset(hexstr, '\0', 3);
		hexstr_idx = 0;
		return 0;
	}

	return -1;
}


void editor_insert_byte(struct editor* e, char x, bool after) {
	int offset = editor_offset_at_cursor(e);
	editor_insert_byte_at_offset(e, offset, x, after);
}

void editor_insert_byte_at_offset(struct editor* e, unsigned int offset, char x, bool after) {
	e->contents = realloc(e->contents, e->content_length + 1);
	if (after && e->content_length) offset++;
	memmove(e->contents + offset + 1, e->contents + offset, e->content_length - offset);
	e->contents[offset] = x;
	e->content_length++;
	e->dirty = true;
}

void editor_replace_byte(struct editor* e, char x) {
	unsigned int offset = editor_offset_at_cursor(e);
	unsigned char prev = e->contents[offset];
	e->contents[offset] = x;
	editor_move_cursor(e, KEY_RIGHT, 1);
	editor_statusmessage(e, STATUS_INFO, "Replaced byte at offset %09x with %02x", offset, (unsigned char) x);
	e->dirty = true;

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

	if (e->mode & (MODE_INSERT | MODE_APPEND)) {
		char out = 0;
		if (editor_read_hex_input(e, &out) != -1) {
		    hexstr[0] = 0; hexstr[1] = 0; hexstr[2] = 0;
		    hexstr_idx = 0;
			editor_insert_byte(e, out, e->mode & MODE_APPEND);
			editor_move_cursor(e, KEY_RIGHT, 1);
		}
		return;
	}

	if (e->mode & (MODE_INSERT_ASCII | MODE_APPEND_ASCII)) {
		char c = read_key();
		if (c == KEY_ESC) {
			editor_setmode(e, MODE_NORMAL); return;
		}
		editor_insert_byte(e, c, e->mode & MODE_APPEND_ASCII);
		editor_move_cursor(e, KEY_RIGHT, 1);
		return;
	}

	if (e->mode & MODE_REPLACE_ASCII) {
		char c = read_key();
		if (c == KEY_ESC) {
			editor_setmode(e, MODE_NORMAL);
			return;
		}

		if (e->content_length > 0) {
			editor_replace_byte(e, c);
		} else {
			editor_statusmessage(e, STATUS_ERROR, "File is empty, nothing to replace");
		}
		return;
	}

	if (e->mode & MODE_REPLACE) {
		char out = 0;
		if (e->content_length > 0) {
			if (editor_read_hex_input(e, &out) != -1) {
                hexstr[0] = 0; hexstr[1] = 0; hexstr[2] = 0;
                hexstr_idx = 0;
				editor_replace_byte(e, out);
			}
		} else {
			editor_statusmessage(e, STATUS_ERROR, "File is empty, nothing to replace");
		}
		return;
	}

	if (e->mode & MODE_COMMAND) {
		char cmd[INPUT_BUF_SIZE];
		int c = editor_read_string(e, cmd, INPUT_BUF_SIZE);
		if (c == KEY_ENTER && strlen(cmd) > 0) {
			editor_process_command(e, cmd);
		}
		return;
	}

	int c = read_key();
	if (c == -1) {
		return;
	}

	switch (c) {
	case KEY_ESC:    editor_setmode(e, MODE_NORMAL); return;
	case KEY_CTRL_Q: exit(0); return;
	case KEY_CTRL_S: editor_writefile(e); return;
	}

	if (e->mode & MODE_NORMAL)
	{
		switch (c) {
		case KEY_UP:
		case KEY_DOWN:
		case KEY_RIGHT:
		case KEY_LEFT: editor_move_cursor(e, c, 1); break;
		case 'h': editor_move_cursor(e, KEY_LEFT,  1); break;
		case 'j': editor_move_cursor(e, KEY_DOWN,  1); break;
		case 'k': editor_move_cursor(e, KEY_UP,    1); break;
		case 'l': editor_move_cursor(e, KEY_RIGHT, 1); break;
//		case 'a': editor_setmode(e, MODE_APPEND);       return;
//		case 'A': editor_setmode(e, MODE_APPEND_ASCII); return;
//		case 'i': editor_setmode(e, MODE_INSERT);       return;
//		case 'I': editor_setmode(e, MODE_INSERT_ASCII); return;
		case 'r': editor_setmode(e, MODE_REPLACE);      return;
		case 'R': editor_setmode(e, MODE_REPLACE_ASCII);return;
		case ':': editor_setmode(e, MODE_COMMAND);      return;
		case KEY_HOME: e->cursor_x = 1; return;
		case KEY_END:  editor_move_cursor(e, KEY_RIGHT, e->octets_per_line - e->cursor_x); return;
		case KEY_CTRL_U:
		case KEY_PAGEUP:   editor_scroll(e, -(e->screen_rows) + 2); return;
		case KEY_CTRL_D:
		case KEY_PAGEDOWN: editor_scroll(e, e->screen_rows - 2); return;
		}
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
