//TODO: SELECTION + CUT COPY PASTE
//HARD: UNDO REDO
//MAYBE: SYNTAX HIGHLIGHT, BETTER CONFIG, IN APP BUILD


#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h> 
#include <stdlib.h>
#include <sys/ioctl.h>

#define TAB_SPACES 4
#define INTIATE_BUILD_COMMAND "gnome-terminal -- bash -c \"./BUILD; exec bash\""
#define INTIATE_RUN_COMMAND "gnome-terminal -- bash -c \"./RUN; exec bash\""

#define MAX_LINES 1000
#define MAX_COLS 1000

#define TEXT_FG "97"
#define LINE_NUMBERS_FG "94"
#define TEXT_AND_LINE_NUMBERS_BG "44"
#define LINE_CONT_FG "94"
#define LINE_CONT_BG "107"
#define CURSOR_FG "34"
#define CURSOR_BG "107"
#define SELECT_FG "34"
#define SELECT_BG "107"
#define STATUS_BAR_FG "34"
#define STATUS_BAR_BG "107"

char text[MAX_LINES][MAX_COLS];
size_t line_lengths[MAX_LINES];
char clipboard[MAX_LINES][MAX_COLS];

size_t cursor_x = 0;
size_t desired_cursor_x=0;
size_t cursor_y = 0;
size_t select_start_x=0;
size_t select_start_y=0;
size_t select_end_x=0;
size_t select_end_y=0;

int last_filled_line=-1;
size_t scroll_start=0;
size_t screen_width=0;
size_t screen_height=0;

char *filename = NULL;
int is_dirty=1;

typedef enum {
    KEY_NONE,
    KEY_CHAR,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_DELETE,
    KEY_TAB,

    KEY_CTRL_S,
    KEY_CTRL_Q,
    KEY_CTRL_B,
    KEY_CTRL_R,

    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_CTRL_HOME,
    KEY_CTRL_END,
    KEY_CTRL_UP,
    KEY_CTRL_DOWN,
    KEY_CTRL_LEFT,
    KEY_CTRL_RIGHT,

    KEY_SHIFT_UP,
    KEY_SHIFT_DOWN,
    KEY_SHIFT_LEFT,
    KEY_SHIFT_RIGHT,
    KEY_SHIFT_HOME,
    KEY_SHIFT_END,
    KEY_SHIFT_CTRL_UP,
    KEY_SHIFT_CTRL_DOWN,
    KEY_SHIFT_CTRL_LEFT,
    KEY_SHIFT_CTRL_RIGHT,
    KEY_SHIFT_CTRL_HOME,
    KEY_SHIFT_CTRL_END,
    KEY_CTRL_C,
    KEY_CTRL_X,
    KEY_CTRL_V,

    KEY_CTRL_Z,
    KEY_CTRL_Y,
} Key;

typedef struct {
    Key key;
    char ch;
} KeyEvent;

static inline char getch() {
    static struct termios oldt, newt;
    static int initialized = 0;
    if (!initialized) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO | ISIG);
        newt.c_iflag &= ~(IXON | IXOFF);
        initialized = 1;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char ch;
    read(STDIN_FILENO, &ch, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

KeyEvent get_key() {
    KeyEvent ev = { .key = KEY_NONE, .ch = 0 };
    int c = getch();
    switch (c) {
        case 3:  ev.key = KEY_CTRL_C; break;
        case 24: ev.key = KEY_CTRL_X; break;
        case 22: ev.key = KEY_CTRL_V; break;
        case 26: ev.key = KEY_CTRL_Z; break;
        case 25: ev.key = KEY_CTRL_Y; break;
        case 19: ev.key = KEY_CTRL_S; break;
        case 17: ev.key = KEY_CTRL_Q; break;
        case 2:  ev.key = KEY_CTRL_B; break;
        case 18: ev.key = KEY_CTRL_R; break;
        case 10: ev.key = KEY_ENTER; break;
        case 127: ev.key = KEY_BACKSPACE; break;
        case 9:  ev.key = KEY_TAB; break;
        case 27: {
            int c2 = getch();
            if (c2 == 91) {
                int c3 = getch();
                if (c3 >= '0' && c3 <= '9') {
                    int c4 = getch();
                    if (c4 == ';') {
                        int mod = getch() - '0';
                        int c5 = getch();
                        if (mod == 2) {
                            switch (c5) {
                                case 65: ev.key = KEY_SHIFT_UP; break;
                                case 66: ev.key = KEY_SHIFT_DOWN; break;
                                case 67: ev.key = KEY_SHIFT_RIGHT; break;
                                case 68: ev.key = KEY_SHIFT_LEFT; break;
                                case 72: ev.key = KEY_SHIFT_HOME; break;
                                case 70: ev.key = KEY_SHIFT_END; break;
                            }
                        } else if (mod == 5) {
                            switch (c5) {
                                case 72: ev.key = KEY_CTRL_HOME; break;
                                case 70: ev.key = KEY_CTRL_END; break;
                                case 65: ev.key = KEY_CTRL_UP; break;
                                case 66: ev.key = KEY_CTRL_DOWN; break;
                                case 67: ev.key = KEY_CTRL_RIGHT; break;
                                case 68: ev.key = KEY_CTRL_LEFT; break;
                            }
                        } else if (mod == 6) {
                            switch (c5) {
                                case 65: ev.key = KEY_SHIFT_CTRL_UP; break;
                                case 66: ev.key = KEY_SHIFT_CTRL_DOWN; break;
                                case 67: ev.key = KEY_SHIFT_CTRL_RIGHT; break;
                                case 68: ev.key = KEY_SHIFT_CTRL_LEFT; break;
                                case 72: ev.key = KEY_SHIFT_CTRL_HOME; break;
                                case 70: ev.key = KEY_SHIFT_CTRL_END; break;
                            }
                        }
                    } else if (c4 == '~') {
                        switch (c3) {
                            case '1': ev.key = KEY_HOME; break;
                            case '4': ev.key = KEY_END; break;
                            case '5': ev.key = KEY_PAGE_UP; break;
                            case '6': ev.key = KEY_PAGE_DOWN; break;
                            case '3': ev.key = KEY_DELETE; break;
                        }
                    }
                } else {
                    switch (c3) {
                        case 65: ev.key = KEY_ARROW_UP; break;
                        case 66: ev.key = KEY_ARROW_DOWN; break;
                        case 67: ev.key = KEY_ARROW_RIGHT; break;
                        case 68: ev.key = KEY_ARROW_LEFT; break;
                        case 72: ev.key = KEY_HOME; break;
                        case 70: ev.key = KEY_END; break;
                    }
                }
            }
        } break;
        default:
            ev.key = KEY_CHAR;
            ev.ch = c;
            break;
    }
    return ev;
}

static inline int fast_log10(int n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    return 5;
}

static inline void update_screen_dimensions(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    screen_height=(size_t)w.ws_row-2;
    screen_width=(size_t)w.ws_col;
}

static inline void load_file(const char *fname) {
    FILE *file = fopen(fname, "r");
    if (!file) return;
    int line = 0;
    char buffer[MAX_COLS];
    while (fgets(buffer, MAX_COLS, file) && line < MAX_LINES) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        if (len > 0 && buffer[len-1] == '\r') {
            buffer[len-1] = '\0';
            len--;
        }
        strcpy(text[line], buffer);
        line_lengths[line] = len;
        line++;
    }
    fclose(file);
}

static inline void save_file() {
    if (!filename) return;
    FILE *file = fopen(filename, "w");
    if (!file) return;
    for (int i = 0; i <= last_filled_line; i++) {
        if (i > 0) fprintf(file, "\n");
        for (size_t j = 0; j < line_lengths[i]; j++) {
            fputc(text[i][j], file);
        }
    }
    fclose(file);
    is_dirty=0;
}

void draw(){
    printf("\x1b[H");
    int file_empty=1;
    for (int y = MAX_LINES-1; y >= 0; y--) if (line_lengths[y]!=0) {last_filled_line=(int)y; file_empty=0; break;}
    if (file_empty==1) last_filled_line=-1;
    for (size_t y = scroll_start; y < MAX_LINES && y < scroll_start+screen_height; y++) {
        printf("\x1b["LINE_NUMBERS_FG";"TEXT_AND_LINE_NUMBERS_BG"m");
        if ((int)y > last_filled_line && y>cursor_y) {
            printf("\x1b[K~\n");
            continue;
        }
        printf("\x1b[K%ld", y + 1);
        int line_number_padding=fast_log10(last_filled_line+1)-fast_log10(y+1)+1;
        printf("%*c", line_number_padding,' ');
        printf("\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m");
        size_t start=0;
        if (y==cursor_y && cursor_x>screen_width-fast_log10(last_filled_line+1)-3 && line_lengths[y]>screen_width-fast_log10(last_filled_line+1)-3) {printf("\x1b[K\x1B["LINE_CONT_FG";"LINE_CONT_BG"m<\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m"); start=cursor_x-cursor_x%(screen_width-fast_log10(last_filled_line+1)-3);}
        for (size_t x = start; x < line_lengths[y]; ++x) {
            if (x-start>screen_width-fast_log10(last_filled_line+1)-(start==0?3:4)) {printf("\x1b[K\x1B["LINE_CONT_FG";"LINE_CONT_BG"m>\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m"); break;}
            if ((y == cursor_y) && (x == cursor_x)) printf("\x1B["CURSOR_FG";"CURSOR_BG"m%c\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m", text[y][x]);
            else printf("\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m%c", text[y][x]);
        }
        if ((y==cursor_y) && (cursor_x==line_lengths[y])) printf("\x1B["CURSOR_FG";"CURSOR_BG"m%c\x1b["TEXT_FG";"TEXT_AND_LINE_NUMBERS_BG"m", ' ');
        printf("\n");
    }
    printf("\x1b[K\x1B["STATUS_BAR_FG";"STATUS_BAR_BG"m%ld:%ld",cursor_y+1, cursor_x+1);
    if (filename) printf(" %s", filename);
    int status_bar_padding=screen_width-(fast_log10(cursor_y+1)+fast_log10(cursor_x+1)+(int)strlen(filename)+fast_log10(last_filled_line+2))-3;
    if (is_dirty==1) printf(" *");
    else status_bar_padding+=2;
    printf("%*c", status_bar_padding, ' ');
    printf("\x1B[0m\n");
}

void handle() {
    KeyEvent ev = get_key();
    switch (ev.key) {
        case KEY_CTRL_S:
            save_file();
            break;
        case KEY_CTRL_Q:
            printf("\x1b[2J\x1b[H");
            exit(0);
        case KEY_CTRL_B:
            system(INTIATE_BUILD_COMMAND);
            break;
        case KEY_CTRL_R:
            system(INTIATE_RUN_COMMAND);
            break;
        case KEY_ARROW_UP:
            if (cursor_y > 0) cursor_y--;
            if (cursor_y < scroll_start) scroll_start--;
            cursor_x = desired_cursor_x > line_lengths[cursor_y] ? line_lengths[cursor_y] : desired_cursor_x;
            break;
        case KEY_CTRL_UP:
            if (scroll_start>0) {scroll_start--; cursor_y--;}
            else if (cursor_y > 0) cursor_y--;
            cursor_x = desired_cursor_x > line_lengths[cursor_y] ? line_lengths[cursor_y] : desired_cursor_x;
            break;
        case KEY_ARROW_DOWN:
            if (last_filled_line > 0 && cursor_y < last_filled_line) cursor_y++;
            else if (cursor_y > last_filled_line) { cursor_y = 0; scroll_start = 0; }
            if (cursor_y >= scroll_start + screen_height) scroll_start++;
            cursor_x = desired_cursor_x > line_lengths[cursor_y] ? line_lengths[cursor_y] : desired_cursor_x;
            break;
        case KEY_CTRL_DOWN:
            if (scroll_start+screen_height-1<last_filled_line) {scroll_start++; cursor_y++;}
            else if (cursor_y < last_filled_line) cursor_y++;
            else if (cursor_y > last_filled_line) { cursor_y = 0; scroll_start = 0; }
            cursor_x = desired_cursor_x > line_lengths[cursor_y] ? line_lengths[cursor_y] : desired_cursor_x;
            break;
        case KEY_ARROW_LEFT:
            if (cursor_x > 0) cursor_x--;
            else if (cursor_x == 0 && cursor_y!=0) {cursor_y--; cursor_x=line_lengths[cursor_y];}
            desired_cursor_x = cursor_x;
            break;
        case KEY_CTRL_LEFT:
            if (cursor_x == 0 && cursor_y!=0) {cursor_y--; cursor_x=line_lengths[cursor_y];}
            else {
                while (cursor_x>0){
                    if (text[cursor_y][cursor_x-1]==' ') cursor_x--;
                    else break;
                }
                while (cursor_x>0){
                    if (text[cursor_y][cursor_x-1]==' ') break;
                    cursor_x--;
                }
            }
            desired_cursor_x = cursor_x;
            break;
        case KEY_ARROW_RIGHT:
            if (cursor_x < line_lengths[cursor_y]) cursor_x++;
            else if (cursor_x == line_lengths[cursor_y] && cursor_y!=last_filled_line) {cursor_y++; cursor_x=0;}
            desired_cursor_x = cursor_x;
            break;
        case KEY_CTRL_RIGHT:
            if (cursor_x == line_lengths[cursor_y] && cursor_y!=last_filled_line) {cursor_y++; cursor_x=0;}
            else {
                while (cursor_x<line_lengths[cursor_y]-1){
                    if (text[cursor_y][cursor_x+1]==' ') break;
                    cursor_x++;
                }
                while (cursor_x<line_lengths[cursor_y]-1){
                    if (text[cursor_y][cursor_x+1]==' ') cursor_x++;
                    else break;
                }
                cursor_x++;
            }
            desired_cursor_x = cursor_x;
            break;
        case KEY_HOME:
            cursor_x = 0;
            desired_cursor_x = cursor_x;
            break;
        case KEY_END:
            cursor_x = line_lengths[cursor_y];
            desired_cursor_x = cursor_x;
            break;
        case KEY_PAGE_UP:
            if (cursor_y!=scroll_start) {cursor_y=scroll_start; break;}
            if (cursor_y>screen_height) {cursor_y-=screen_height; scroll_start=cursor_y;}
            else {cursor_y=0; scroll_start=0;}
            break;
        case KEY_PAGE_DOWN:
            if (cursor_y!=scroll_start+screen_height-1) {cursor_y=scroll_start+screen_height-1; break;}
            if (cursor_y+screen_height>last_filled_line) {cursor_y=last_filled_line; scroll_start=last_filled_line-screen_height+1;}
            else {cursor_y+=screen_height; scroll_start=cursor_y-screen_height+1;}
            break;
        case KEY_CTRL_HOME:
            cursor_x=0; cursor_y=0; scroll_start=0;
            break;
        case KEY_CTRL_END:
            cursor_x=line_lengths[last_filled_line];
            cursor_y=last_filled_line;
            if ((scroll_start+screen_height-1)<last_filled_line) scroll_start=last_filled_line-screen_height+1;
            break;
        case KEY_DELETE:
            if (cursor_x < line_lengths[cursor_y]) {
                for (size_t i = cursor_x; i < line_lengths[cursor_y] - 1; i++) {
                    text[cursor_y][i] = text[cursor_y][i + 1];
                }
                line_lengths[cursor_y]--;
                is_dirty=1;
            }
            break;
        case KEY_ENTER:
            if (last_filled_line == MAX_LINES - 1) break;
            if (cursor_y < MAX_LINES - 1) {
                for (size_t y = MAX_LINES - 1; y > (int)cursor_y; y--) {
                    memcpy(text[y], text[y - 1], MAX_COLS);
                    line_lengths[y] = line_lengths[y - 1];
                }
                size_t remaining = line_lengths[cursor_y] - cursor_x;
                if (remaining > 0) {
                    memcpy(text[cursor_y + 1], &text[cursor_y][cursor_x], remaining);
                    line_lengths[cursor_y + 1] = remaining;
                } else {
                    memset(text[cursor_y + 1], 0, MAX_COLS);
                    line_lengths[cursor_y + 1] = 0;
                }
                line_lengths[cursor_y] = cursor_x;
                memset(&text[cursor_y][cursor_x], 0, MAX_COLS - cursor_x);
                cursor_y++;
                cursor_x = 0;
            }
            if (cursor_y >= scroll_start + screen_height) scroll_start++;
            is_dirty=1;
            break;
        case KEY_BACKSPACE:
            if (cursor_x > 0) {
                cursor_x--;
                for (size_t i = cursor_x; i < line_lengths[cursor_y] - 1; i++) {
                    text[cursor_y][i] = text[cursor_y][i + 1];
                }
                text[cursor_y][--line_lengths[cursor_y]] = 0;
            } else if (cursor_y > 0) {
                size_t prev = cursor_y - 1;
                size_t len = line_lengths[prev];
                if (len + line_lengths[cursor_y] < MAX_COLS) {
                    memcpy(&text[prev][len], text[cursor_y], line_lengths[cursor_y]);
                    line_lengths[prev] += line_lengths[cursor_y];
                    cursor_y = prev;
                    cursor_x = len;
                    for (size_t y = cursor_y + 1; y < MAX_LINES - 1; y++) {
                        memcpy(text[y], text[y + 1], MAX_COLS);
                        line_lengths[y] = line_lengths[y + 1];
                    }
                    memset(text[MAX_LINES - 1], 0, MAX_COLS);
                    line_lengths[MAX_LINES - 1] = 0;
                    if (cursor_y < scroll_start) scroll_start--;
                }
            }
            if (scroll_start < 0) scroll_start = 0;
            is_dirty=1;
            break;
        case KEY_TAB:
            if (line_lengths[cursor_y] + TAB_SPACES < MAX_COLS && cursor_x + TAB_SPACES <= MAX_COLS) {
                for (int i = 0 ; i < TAB_SPACES; i++) {
                    for (size_t j = line_lengths[cursor_y]; j > cursor_x; j--) {
                        text[cursor_y][j] = text[cursor_y][j - 1];
                    }
                    text[cursor_y][cursor_x++] = ' ';
                    line_lengths[cursor_y]++;
                }
                is_dirty=1;
            }
            break;
        case KEY_CHAR:
            if (line_lengths[cursor_y] < MAX_COLS) {
                for (size_t i = line_lengths[cursor_y]; i > cursor_x; i--) {
                    text[cursor_y][i] = text[cursor_y][i - 1];
                }
                text[cursor_y][cursor_x++] = ev.ch;
                line_lengths[cursor_y]++;
                is_dirty=1;
            }
            break;

        default:
            break;
    }
    if (cursor_y > MAX_LINES - 1) cursor_y = last_filled_line;
    if (cursor_x > line_lengths[cursor_y]) cursor_x = line_lengths[cursor_y];
}

int main(int argc, char *argv[]) {
    for (int r = 0; r < MAX_LINES; ++r) {
        memset(text[r], 0, MAX_COLS);
        line_lengths[r] = 0;
    }
    if (argc > 1) {
        filename = argv[1];
        load_file(filename);
    }
    else return 1;
    printf("\x1b[2J\x1b[H");
    while (1){
        update_screen_dimensions();
        draw();
        handle();
    }
}