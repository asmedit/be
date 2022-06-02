#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "../buffer.h"
#include "../editor.h"
#include "../terminal.h"

#include "hex.h"

int  hexstr_idx = 0;
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

void editor_cursor_at_offset(struct editor* e, int offset, int* x, int* y) {
	*x = offset % e->octets_per_line + 1;
	*y = offset / e->octets_per_line - e->line + 1;
}

int editor_offset_at_cursor(struct editor* e) {
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


void editor_render_hex(struct editor* e, struct charbuf* b) {

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

void editor_process_keypress_hex(struct editor* e) {

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
		case 'd': editor_setmode(e, MODE_DASM);         return;
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

