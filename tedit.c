#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdio.h>


struct termios orig_termois;

void disableRawMode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termois);
}

void enableRawMode(){
    tcgetattr(STDIN_FILENO, &orig_termois);
    atexit(disableRawMode);
    struct termios obj = orig_termois;
    obj.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
    obj.c_cflag &= |(CS8);
    obj.c_oflag &= ~(OPOST);
    obj.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &obj);
}

int main(){

    enableRawMode();
    
    char c;
    while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        if(iscntrl(c)){
            printf("%d\r\n", c);
        }else{
            printf("%d('%c')\r\n",c,c);
        }
    }
    return 0;
}