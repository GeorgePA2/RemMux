#pragma once
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
#include <sys/stat.h>
#include <string>
#include <vector>
#include <fcntl.h>
#include <panel.h>

using namespace std;

class my_windows{
    private:
    typedef struct win_size_info{
        int startx_inside, starty_inside;
        int inside_height, inside_lenght;
        int startx_border, starty_border;
        int border_height, border_lenght;
    }win_size_info;
    vector<WINDOW*> inside_box;
    vector<WINDOW*> border;
    vector<win_size_info> sizes;
    int current_window=0;
    int windows_opened=0;
    int max_size=6;
    int par = 1;
    int impar = 1;

    public:
    my_windows();
    ~my_windows();
    int Get_CurrentW();
    WINDOW* Get_CurrentWindow();
    int Get_nrWindows();
    void Change_CurrWin(int nr);
    int Get_MaxSize();
    void Change_MaxSize(int new_size);
    void Add_Window();
    void Create_Window(int height, int width, int start_y, int start_x);
    void RestoreWindow(int position);
    void CreateWindowHistory(int position, string& msg);
    void log_history(string &msg);
    void resize_win();


};