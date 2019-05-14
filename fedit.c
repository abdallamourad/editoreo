/*** includes ***/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>

/*** difines***/

#define CTRL_KEY(k)     ((k) & 0x1f)

/*** data ***/

struct editorConfig {
    int screenrows;
    int screencols;
    struct termios ori_termios;
};

struct editorConfig E;

/*** terminal ***/

void die(const char*s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    perror(s);
    exit(1);
}

void disableRawMode(){
    // return the terminal to its original state when exit
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.ori_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    // get all terminal attributes
    if(tcgetattr(STDIN_FILENO, &E.ori_termios) == -1) die("tcgetattr");
    // when exit call disableRawMode funstion
    atexit(disableRawMode);

    // new termios struct to change the attributes.
    struct termios raw = E.ori_termios;
    
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    
    // VMIN is to set what is the minimunm bytes it takes
    // VTIME is to set the after what time it would issue a timeout. 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 15;
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

char editorReadKey() {
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) == 1){
        if(nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** input ***/

void editorProcessKeyPress() {
    char c = editorReadKey();

    if(c == CTRL_KEY('q')){
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
    }
}

/*** output ***/

void editorDrawRows(){
    int y;
    for(y = 0; y < E.screenrows; y++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/

void initEditor() {
    if(getWindowSize(&E.screenrows, &E.screencols) == -1) 
        die("getWindowSize");
}
int main(){
    enableRawMode();
    initEditor();

    while(1){
        editorRefreshScreen();
        editorProcessKeyPress();
    }
    return 0;
}
