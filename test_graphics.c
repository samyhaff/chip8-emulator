#include <curses.h>

int main(int argc, char **argv) {
    initscr();
    cbreak();
    noecho();
    
    WINDOW *main = newwin(32, 64, 0, 0);
    refresh();

    box(main, 0, 0);
    mvwprintw(main, 30, 1, "#");
    mvwprintw(main, 20, 1, "#");
    mvwprintw(main, 2, 10, "#");
    wrefresh(main);

    int c = getch();

    endwin();
}
