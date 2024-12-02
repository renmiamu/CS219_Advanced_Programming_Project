#include <ncurses.h>

int main(){
    initscr();
    printw("hello world");
    refresh();
    getchar();
    endwin();
    return 0;
}