#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdio.h>
#include<errno.h>
#include<sys/ioctl.h>


#define CTRL_KEY(k) ((k) & 0x1f)      // turns off bit 7, 6 and 5 of the char

struct editorConfig{
    int screenrows;
    int screencols;
    struct termios orig_termois;
};

struct editorConfig E;

void die(char *s){
    write(STDOUT_FILENO, "\x1b[2J",4);  
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    perror(s);
    exit(1);
}


char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1){
        if(nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}


int getCursorPosition(int *rows, int *cols){
    char buf[32];
    unsigned int i = 0;

    if(write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    
    while(i < sizeof(buf) -1){
        if(read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if(buf[i] == 'R')
            break;
        i++;
    }

    buf[i] = '\0';
    
    if(buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if(sscanf(&buf[2],"%d;%d", rows, cols) != 2)
        return -1;

    return 0;
}


int getWindowSize(int *rows, int *cols){
    struct winsize wbuf;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &wbuf) == -1 || wbuf.ws_col == 0){
        if(write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition(rows, cols);
    }
    else{
        *cols = wbuf.ws_col;
        *rows = wbuf.ws_row;
        return 0;
    }
}


void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termois) == -1)
        die("tcsetattr");
}


void enableRawMode(){
    if(tcgetattr(STDIN_FILENO, &E.orig_termois) == -1)
        die("tcgetsttr");
    atexit(disableRawMode);
    struct termios obj = E.orig_termois;
    obj.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    obj.c_cflag |= (CS8);
    obj.c_oflag &= ~(OPOST);
    obj.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    obj.c_cc[VMIN] = 0;
    obj.c_cc[VTIME] = 1;
    
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &obj) == -1)
        die("tcsetattr");

}


void editorProcessKeypress(){
    char c = editorReadKey();
    switch(c){
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J",4);  
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}


void editorDrawRows(){
    int y;
    for(y = 0; y < E.screenrows-1; y++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
    write(STDOUT_FILENO, "~", 1);
}


void editorRefreshScreen(){
    write(STDOUT_FILENO, "\x1b[2J",4);  // 033 is 27. Every Escape sequence starts with 27.
                                        // We can also use x1b in place of 033. It gets 
                                        // translated to 27.
    write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor at the top-left of screen

    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void initEditor(){
    if(getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}

int main(){

    enableRawMode();
    initEditor();
    
    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}