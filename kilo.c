/*** includes ***/

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

/*** data ***/

// save the original attribures to set them when exit 
struct termios ori_termios;

/*** terminal ***/

void die(const char*s) {
    perror(s);
    exit(1);
}

void disableRawMode(){
    // return the terminal to its original state when exit
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &ori_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){
    // get all terminal attributes
    if(tcgetattr(STDIN_FILENO, &ori_termios) == -1) die("tcgetattr");
    // when exit call disableRawMode funstion
    atexit(disableRawMode);

    // new termios struct to change the attributes.
    struct termios raw = ori_termios;
    
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

/*** init ***/
int main(){
    enableRawMode();

    while(1){
        char c = '\0';
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if(iscntrl(c)){
            printf("%d\r\n", c);
        } else {
            printf("%c\r\n", c);
        }
        if (c == 'q') break;
    }
    return 0;
}