#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <cstring>
#include <ncurses.h>
#include <string>
#include <vector>
#include <fcntl.h>

using namespace std;

WINDOW* create_window(int inaltime, int lungime, int y_coord, int x_coord){
    WINDOW* ret_win = newwin(inaltime, lungime, y_coord, x_coord);
    box(ret_win, 0, 0);
    wrefresh(ret_win);
    return ret_win;

}


int main(){
    initscr();
    noecho();
    cbreak();
    if(has_colors){
        start_color();
    }
    init_pair(1, COLOR_BLACK, COLOR_CYAN);
    int enable_arrow=0;
    int x, y;
    getyx(stdscr, y, x);
    int key;
    char buf[4096];
    string msg;
    vector<WINDOW*> ferestre_deschise;
    int fd = open("logs.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd==-1){return -1;}
    key = 0;
    while((key = getch()) != 'q'){

        WINDOW* new_window = create_window(getmaxy(stdscr)/2, getmaxx(stdscr)/2, y, x);
        scrollok(new_window, true);
        idlok(new_window, true);
        wbkgd(new_window, COLOR_PAIR(1));
        keypad(new_window, true);
        wmove(new_window, 1, 1);
        ferestre_deschise.push_back(new_window);
        //wprintw(new_window, "%d %d\n", getmaxx(stdscr), getmaxy(stdscr));

        msg = "Y MAXIM: " + to_string(getmaxy(stdscr)) + "\n" + "X MAXIM: " + to_string(getmaxx(stdscr)) + "\n" + "Y ESTE: " + to_string(y) + "\n" + "X ESTE: " + to_string(x) + "\n";
        write(fd, msg.c_str(), msg.size());
        msg.clear();

       // wprintw(new_window, "%d %d\n", y, x);
        wrefresh(new_window);
        int win_y, win_x;
        getyx(new_window, win_y, win_x);
        mvwprintw(new_window,win_y+1, 2, "Goodbye!\n");
        wrefresh(new_window);
        x += getmaxx(stdscr)/2;
        if(x+getmaxx(stdscr)/2>getmaxx(stdscr)){
            x = 0;
            y+= getmaxy(stdscr)/2;
        }
        if(y+getmaxy(stdscr)/2> getmaxy(stdscr)){
            msg = "Y MAXIM: " + to_string(getmaxy(stdscr)) + "\n" + "X MAXIM: " + to_string(getmaxx(stdscr)) + "\n" + "Y ESTE: " + to_string(y) + "\n" + "X ESTE: " + to_string(x) + "\n";
            write(fd, msg.c_str(), msg.size());
            msg.clear();
            break;
        }
        key = getch();

    }

    int current_window = 0;

    msg = "Am intrat in loop!\n";
    write(fd, msg.c_str(), msg.size());
    msg.clear();
    do{

        int win_y, win_x;
        getyx(ferestre_deschise[current_window], win_y, win_x);

        switch (key){
        
        case '\t':
        enable_arrow = 1;
        if(current_window+1<ferestre_deschise.size()){
            current_window++;
        }
        else{
            current_window = 0;
        }
        wmove(ferestre_deschise[current_window], 2, 2);
        wrefresh(ferestre_deschise[current_window]);
        msg = "We tabbed!\n";
        write(fd, msg.c_str(), msg.size());
        msg.clear();
        break;

        case KEY_LEFT:
        if(enable_arrow==0){
            msg = "WE CAN'T USE ARROWS AT THE MOMENT!";
            write(fd, msg.c_str(), msg.size());
            msg.clear();
            break;}
        if(current_window-1<0){
            current_window = ferestre_deschise.size();

        }
        else{
            current_window--;
        }
        wmove(ferestre_deschise[current_window], 2, 2);
        wrefresh(ferestre_deschise[current_window]);
        msg = "We tabbed!\n";
        write(fd, msg.c_str(), msg.size());
        msg.clear();
        break;

        case KEY_RIGHT:
        if(enable_arrow==0){
            msg = "WE CAN'T USE ARROWS AT THE MOMENT!";
            write(fd, msg.c_str(), msg.size());
            msg.clear();
            break;}
        if(current_window+1<ferestre_deschise.size()){
            current_window++;

        }
        else{
            current_window = 0;
        }
        wmove(ferestre_deschise[current_window], 2, 2);
        wrefresh(ferestre_deschise[current_window]);
        msg = "We tabbed!\n";
        write(fd, msg.c_str(), msg.size());
        msg.clear();
        break;

        case 10:
        noecho();
        enable_arrow = 0;
        wmove(ferestre_deschise[current_window], win_y, win_x+2);
        wgetstr(ferestre_deschise[current_window], buf);
        wprintw(ferestre_deschise[current_window], "%s\n", buf);
        wrefresh(ferestre_deschise[current_window]);
        //noecho();
        break;
        }
        msg = "We are in the beam!\n";
        write(fd, msg.c_str(), msg.size());
        msg.clear();
        

    }while((key = getch()) != KEY_F(1));

    remove("logs.txt");
    close(fd);
    endwin();
    return 0;

}