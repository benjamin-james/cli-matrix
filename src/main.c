#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/ioctl.h"
#include "signal.h"
#include "time.h"

#include "array.h"

#define TICK_USEC 30000
#define BG_DEFAULT 49
#define BG_BLACK 40
#define BG_COLOR BG_DEFAULT /*could use either 49 (default bg) or 40 (black bg) */
#define MAX_RAINDROPS 1000

struct drop {
	int x, y;
	int len, max_len;
	int ticks_left;
};

struct winsize w;
volatile bool is_running = 1;

int step(struct drop *d);
void moveCursorTo(int x, int y);
void colorPrint(char c, int fg, int bg, int attr);
void showCursor(int state);
void sighandler(int signum);
void getSize(int *row, int *col);
struct drop *newDrop();

int main(int argc, char **argv)
{
	srand(time(0));
	int i;
	signal(SIGINT, sighandler);
	showCursor(0);

	struct array *a = NULL;
	for (a = array_create(10,NULL); is_running; usleep(TICK_USEC)) {
		for (i = 0; i < a->size; i++) {
			if (step((struct drop *)(a->buffer[i])) < 0) {
				free(array_remove(a, i--));
			}
		}
		if (a->size < MAX_RAINDROPS) {
			array_append(a, newDrop());
			if (rand() % 2)
				array_append(a, newDrop());
		}
		fflush(stdout);
	}
	while (a->size > 0) {
		free(array_remove(a,a->size-1));
	}
	array_destroy(a);
	i = system("clear");
	fflush(stdout);
	showCursor(1);
	return i;
}

int step(struct drop *d)
{
	int rows, cols;
	char c;
	getSize(&rows, &cols);
	if (!d || d->y > rows || d->x > cols)
		return -1;
	if (d->ticks_left <= 0 && d->len <= 0) {
		moveCursorTo(d->y, d->x);
		colorPrint(' ', 30, BG_COLOR, 1);
		return -1;
	}
	if ((d->len == d->max_len || (d->len > 0 && d->ticks_left <= 0)) && d->y <= rows) {
		moveCursorTo(d->y++, d->x);
		colorPrint(' ', 30, BG_COLOR, 1);
		d->len--;
	}
	if (d->len < d->max_len && d->ticks_left > 0 && d->y + d->len <= rows) {
		c = rand() % 57 + 33;
		moveCursorTo(d->y + d->len++, d->x);
		if (rand() % 8)
			colorPrint(c, 32, BG_COLOR, 2);
		else
			colorPrint(c, 32, BG_COLOR, 1);
		moveCursorTo(d->y + d->len, d->x);
		colorPrint(c, 37, BG_COLOR, 1);
	}
	d->ticks_left--;
	return 0;
}

struct drop *newDrop()
{
	int rows, cols;
	getSize(&rows, &cols);
	struct drop *d = malloc(sizeof(struct drop));
	if (rand() % 2)
		d->y = 0;
	else
		d->y = rand() % (rows+1);
	d->x = rand() % (cols+1);
	d->len = 0;
	d->max_len = rand() % (3 * rows / 2) + 4;
	if (d->max_len >= rows)
		d->max_len = rows - 1;
	d->ticks_left = rand() % 20 + d->max_len - 5;
	if (d->ticks_left < 0)
		d->ticks_left = d->max_len;
	return d;
}

void moveCursorTo(int x, int y)
{
	printf("%c[%d;%df",0x1B,x,y);
}

void colorPrint(char c, int fg, int bg, int attr)
{
	printf("%c[%d;%d;%dm%c\b\e[0m", 0x1B, attr, fg, bg, c);
}

void showCursor(int state)
{
	if (state) {
		fputs("\e[" "?25h", stdout);
	} else {
		fputs("\e[" "?25l", stdout);
	}
}

void sighandler(int signum)
{
	is_running = 0;
}

void getSize(int *row, int *col)
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	*row = w.ws_row;
	*col = w.ws_col;
}
