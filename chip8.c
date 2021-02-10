#include <curses.h>

int main(int argc, char **argv) {
    initscr();
    
    WINDOW *main = newwin(32, 64, 0, 0);
    refresh();

    box(main, 0, 0);
    // mvwprintw(main, 30, 1, "Test");
    wrefresh(main);

    int c = getch();

    endwin();
}
