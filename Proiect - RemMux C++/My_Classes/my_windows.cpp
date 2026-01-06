#include "./my_windows.h"
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
#include "my_windows.h"

void my_windows::ratio_creation()
{

  if(!ratios.empty()){
    ratios.clear();
  }

  win_ratio ratie_0{
    .x_ratio = 0,
    .y_ratio = 0,
    .h_ratio = float(getmaxy(this->border[0])) / float(getmaxy(stdscr)),
    .l_ratio = float(getmaxx(this->border[0])) / float(getmaxx(stdscr))
  };
  string msg;
  log_history(msg = to_string(ratie_0.x_ratio) + " " + to_string(ratie_0.y_ratio)+ "\n");
  this->ratios.push_back(ratie_0);
  for(int i=1;i<windows_opened;i++){

    win_ratio ratie{
      .x_ratio = float(getbegx(this->border[i])) / float(getmaxx(stdscr)),
      .y_ratio = float(getbegy(this->border[i])) / float(getmaxy(stdscr)),
      .h_ratio = float(getmaxy(this->border[i])) / float(getmaxy(stdscr)),
      .l_ratio = float(getmaxx(this->border[i])) / float(getmaxx(stdscr))
    };
    this->ratios.push_back(ratie);
    log_history(msg = to_string(ratie.x_ratio) + " " + to_string(ratie.y_ratio)+ "\n");
  }
}

my_windows::my_windows()
{
    WINDOW* new_window = newwin(getmaxy(stdscr), getmaxx(stdscr), getbegy(stdscr), getbegx(stdscr));
    box(new_window, 0, 0);
    WINDOW* innernew_window = newwin(getmaxy(stdscr)-2, getmaxx(stdscr)-2, getbegy(stdscr)+1, getbegx(stdscr)+1);
    //box(innernew_window, '=', '+');
    idlok(innernew_window, true);
    scrollok(innernew_window, true);
    keypad(innernew_window, true);
    wrefresh(new_window);
    this->border.push_back(new_window);
    this->inside_box.push_back(innernew_window);
    this->windows_opened = 1;
    this->current_window = 0;
    this->max_size = 6;
  
    wmove(innernew_window, 0, 0);
    wprintw(inside_box[current_window], "Press <<Enter>> to start typing, '+' to create a new window <<tab>> to switch between windows or 'q' to quit!\n");
    string logs =  "Press <<Enter>> to start typing, '+' to create a new window <<tab>> to switch between windows or 'q' to quit!\n";
    CreateWindowHistory(this->current_window, logs);
    wrefresh(inside_box[current_window]);
    log_history(logs = "Avem " + to_string(windows_opened) + " ferestre active!\n");
    //this->sizes.push_back{}

}

my_windows::~my_windows()
{
    for(auto x:this->border){
        delwin(x);
    }
    for(auto x:this->inside_box){
        delwin(x);
    }
    string file_history;
    for(int i=0;i<=windows_opened;i++){
      file_history = "./Client_logs/window" + to_string(getpid()) + "_" + to_string(i);
      remove(file_history.c_str());
    }
}

int my_windows::Get_CurrentW()
{
    return current_window;
}

WINDOW *my_windows::Get_CurrentWindow()
{
    return inside_box[current_window];
}

int my_windows::Get_nrWindows()
{
    return this->windows_opened;
}

void my_windows::Change_CurrWin(int nr)
{
  current_window = nr;
}
int my_windows::Get_MaxSize()
{
    return this->max_size;
}

void my_windows::Change_MaxSize(int new_size)
{
    this->max_size = new_size;
}

void my_windows::Add_Window()
{
    int maxx_win, maxy_win, init_x, init_y;
    string logs;
    if((windows_opened<max_size) && (windows_opened >=1)){
    
      log_history(logs = "Avem " + to_string(windows_opened) + " ferestre active!\n");
      maxx_win = getmaxx(border[windows_opened-1]);
      maxy_win = getmaxy(border[windows_opened-1]);
      init_x = getbegx(border[windows_opened-1]);
      init_y = getbegy(border[windows_opened-1]);
      
      if(windows_opened%2==0){
      Create_Window(maxy_win/2, maxx_win, init_y+maxy_win/2, init_x);
      }
      else{
        Create_Window(maxy_win, maxx_win/2, init_y, init_x+maxx_win/2);
      }
      current_window=windows_opened-1;
      
      CreateWindowHistory(current_window, logs="");
  
  
      if((windows_opened-2)%2==0){
      // wborder(active_windows[active_windows.size()-2], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      
      werase(border[windows_opened-2]);
      wclear(border[windows_opened-2]);
      touchwin(border[windows_opened-2]);
      wrefresh(border[windows_opened-2]);
      werase(inside_box[windows_opened-2]);
      wclear(inside_box[windows_opened-2]);
      touchwin(inside_box[windows_opened-2]);
      wrefresh(inside_box[windows_opened-2]);

      wmove(border[windows_opened-2], 1, 1);
      wmove(inside_box[windows_opened-2], 1, 1);
      wresize(border[windows_opened-2], maxy_win, maxx_win/2);
      wresize(inside_box[windows_opened-2], maxy_win-2, (maxx_win/2)-2);
      // wresize(border[windows_opened-1], maxy_win, maxx_win/2);
      // wresize(inside_box[windows_opened-1], maxy_win-2, (maxx_win/2)-2);


      //mvwprintw(active_windows[active_windows.size()-2], getmaxy(active_windows[active_windows.size()-2])-1, getmaxx(active_windows[active_windows.size()-2])/2, "%d", (int)active_windows.size()-1);
      //wmove(new_window, 1, 1);
      }
      else{
       // wborder(active_windows[active_windows.size()-2], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
       werase(border[windows_opened-2]);
       wclear(border[windows_opened-2]);
       touchwin(border[windows_opened-2]);
       wrefresh(border[windows_opened-2]);
       werase(inside_box[windows_opened-2]);
       wclear(inside_box[windows_opened-2]);
       touchwin(inside_box[windows_opened-2]);
       wrefresh(inside_box[windows_opened-2]);

       wresize(border[windows_opened-2], maxy_win/2, maxx_win);
       wresize(inside_box[windows_opened-2], (maxy_win/2)-2, maxx_win-2);
      //  wresize(border[windows_opened-1], maxy_win/2, maxx_win);
      //  wresize(inside_box[windows_opened-1], (maxy_win/2)-2, maxx_win-2);
        //mvwprintw(active_windows[active_windows.size()-2], getmaxy(active_windows[active_windows.size()-2])-1, getmaxx(active_windows[active_windows.size()-2])/2, "%d", (int)active_windows.size()-1);
        //wmove(new_window, 1, 1);
       
      }
    
      //box(Get_CurrentWindow(), '=', '+');
      box(border[windows_opened-1], 0, 0);
      box(border[windows_opened-2], 0, 0);
      RestoreWindow(windows_opened-2);
      //box(inside_box[windows_opened-2], '=', '+');
      wrefresh(border[windows_opened-2]);
      wrefresh(inside_box[windows_opened-2]);
      wrefresh(border[windows_opened-1]);
      wrefresh(inside_box[windows_opened-1]);

    }
    ratio_creation();

}

void my_windows::Create_Window(int height, int width, int start_y, int start_x)
{
    WINDOW* new_window = newwin(height, width, start_y, start_x);
    WINDOW* innernew_window = newwin(height-2, width-2, start_y+1, start_x+1);
    box(new_window, 0, 0);
    //box(innernew_window, '-', '+');
    idlok(innernew_window, true);
    scrollok(innernew_window, true);
    keypad(innernew_window, true);
    wmove(innernew_window, 0, 0);
    wrefresh(new_window);
    wrefresh(innernew_window);
    this->border.push_back(new_window);
    this->inside_box.push_back(innernew_window);



    this->windows_opened++;
    doupdate();
    string msg;
    log_history(msg = "Avem " + to_string(windows_opened) + "ferestre deschise si fereastra curenta este: " + to_string(current_window) + "\n" );
}

void my_windows::RestoreWindow(int position)
{
    string istoric_fereastra = "./Client_logs/window" + to_string(getpid()) + "_" + to_string(position);
    string err;
    struct stat stbuf;
    wmove(inside_box[position], 0, 0);
    int fd = open(istoric_fereastra.c_str(), O_RDONLY);
    if(fd==-1){
      //perror("ERORARE LA DESCHIDEREA FISIERULUI!");
      log_history(err= "A avut loc o eroare la deschiderea fisierului window" + to_string(getpid()) + "_" + to_string(position) +"\n");
      return;
    }
    if(lstat(istoric_fereastra.c_str(), &stbuf)==-1){
      //perror("EROARE LA lstat!");
      log_history(err = "EROARE LA lstat!");
      return;
    }
  
    int file_size = stbuf.st_size;
    char istory[file_size+1];
  
    if(read(fd, istory, file_size)==-1){
      //perror("EROARE LA CITIRE DIN FISIER!");
      log_history(err = "EROARE LA CITIREA DIN FISIERUL window"+to_string(position)+"\n");
      return;
    }
    istory[file_size] = '\0';
    wprintw(inside_box[position], "%s", istory);
    istoric_fereastra.clear();
    err.clear();
    wrefresh(border[position]);
    wrefresh(inside_box[position]);
    close(fd);
}



void my_windows::CreateWindowHistory(int position, string &msg)
{
        string nume_fisier = "./Client_logs/window" + to_string(getpid()) + "_" + to_string(position);
        string toerrishuman;
        int fd = open(nume_fisier.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
        if(fd==-1){
          //perror("ERORARE LA CITIREA/CREEREA FISIERULUI!");
          log_history(toerrishuman = "A avut loc o eroare la creerea fisierului window" + to_string(position)+"\n");
          return;
        }
        if(write(fd, msg.c_str(), msg.size())==-1){
          //perror("EROARE LA SCRIEREA PE FISIER!");
          log_history(toerrishuman = "A avut loc o eroare la scrierea pe fisierul window" + to_string(position)+"\n");
          return;
        }
        msg.clear();
        nume_fisier.clear();
        toerrishuman.clear();
        close(fd);
}

void my_windows::log_history(string &msg)
{
    int fd = open("./my_logs", O_RDWR | O_CREAT | O_APPEND, 0666);
    if(fd==-1){
        //perror("EROARE LA DESCHIDEREA FISIERULUI!");
        return;
    }
    if(write(fd, msg.c_str(), msg.size())<=-1){
          return;
    }
    msg.clear();
    close(fd);    
}

void my_windows::resize_win()
{
  endwin();
  initscr();   
  cbreak();   
  nonl();   
  refresh();
  string rsz = "RESIZE!!";
  
  for(auto x:this->border){
    werase(x);
    wclear(x);
    touchwin(x);
    wrefresh(x);
    delwin(x);
  }
  this->border.clear();
  for(auto x:this->inside_box){
    werase(x);
    wclear(x);
    touchwin(x);
    wrefresh(x);
    delwin(x);
  }
  this->inside_box.clear();



  int prev_wincount = windows_opened;

  windows_opened = 0;
  current_window = -1;

  log_history(rsz = "INFORMATII: \n size="
  + to_string(getmaxx(stdscr)) + ", " + to_string(getmaxy(stdscr)) + "\n");

  int start_x = 0;
  int start_y = 0;

  for(int i=0;i<prev_wincount;i++){
    current_window = i;
    // if((i%2==1) && (i>0)){

    //   this->sizes[i].startx_border = getbegx(this->border[i-1]) + (getmaxx(stdscr) / this->sizes[i-1].border_height);
    //   this->sizes[i].starty_border = getbegy(this->border[i-1]);
    // }
    // else if((i%2==0) && (i>0)){
    //   this->sizes[i].startx_border = getbegx(this->border[i-1]);
    //   this->sizes[i].starty_border = getbegy(this->border[i-1]) + (getmaxy(stdscr) / this->sizes[i-1].border_lenght);

    // }

    // Create_Window(getmaxy(stdscr) / this->sizes[i].border_height, getmaxx(stdscr) / this->sizes[i].border_lenght, this->sizes[i].starty_border, this->sizes[i].startx_border);
    start_x = getmaxx(stdscr) * this->ratios[i].x_ratio;
    start_y = getmaxy(stdscr) * this->ratios[i].y_ratio;

    Create_Window(getmaxy(stdscr) * this->ratios[i].h_ratio, getmaxx(stdscr) * this->ratios[i].l_ratio, start_y, start_x);
    RestoreWindow(i);
    // log_history(rsz = "INFORMATII: \n current window: " + to_string(i) + "\n start= " + to_string(this->sizes[i].startx_border) + ", " + to_string(this->sizes[i].starty_border) + "\n size="
    //   + to_string(this->sizes[i].border_height) + ", " + to_string(this->sizes[i].border_lenght) + "\n");

  }
  refresh();
  log_history(rsz);
}


