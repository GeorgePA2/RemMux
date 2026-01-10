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


int my_windows::return_current_line(int window)
{
    int curr_line, aux;
    getyx(inside_box[window], curr_line, aux);
    return curr_line;
}

int my_windows::return_totalLines(int window)
{
  enable_log_history = true;
  int line_count = 0;

  int i =0;
  for(const auto & x:Window_History[window]){
    string log = x;
    log_history(log += to_string(i) + "\n");
    line_count += x.size() / (int)getmaxx(inside_box[window]) + 1;
    i++;
  }
  return line_count;

}

void my_windows::splitup_MSG(vector<string> &lines, string msg, int curr_window)
{
  string new_line;
  for(int i=0;i<(int)msg.size();i++){
    new_line += msg[i];
    if(msg[i]=='\n'){
      lines.push_back(new_line);
      new_line.clear();
    }
    if((int)new_line.size()==getmaxx(inside_box[curr_window])){
      lines.push_back(new_line);
      new_line.clear();
    }
  }
  if(!new_line.empty()){
    lines.push_back(new_line);
    new_line.clear();
  }
}

void my_windows::splitup_MSG_Restore(vector<string> &lines, string &new_line, string msg, int curr_window)
{
  for(int i=0;i<(int)msg.size();i++){
    new_line += msg[i];
    if(msg[i]=='\n'){
      lines.push_back(new_line);
      new_line.clear();
    }
    if((int)new_line.size()==getmaxx(inside_box[curr_window])){
      lines.push_back(new_line);
      new_line.clear();
    }
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
    this->enable_log_history = true;
  
    wmove(innernew_window, 0, 0);
    wprintw(inside_box[current_window], "Press <<Enter>> to start typing, '+' to create a new window <<tab>> to switch between windows or 'q' to quit!\n");
    string logs =  "Press <<Enter>> to start typing, '+' to create a new window <<tab>> to switch between windows or 'q' to quit!\n";

    this->total_lines.push_back(0);

    this->total_chars.push_back((int)logs.size());

    CreateWindowHistory(this->current_window, logs);
    wrefresh(inside_box[current_window]);
    this->current_pos.push_back(return_current_line(0));
    log_history(logs = "Avem " + to_string(windows_opened) + " ferestre active!\n");
    //this->sizes.push_back{}

    if(!ratios.empty()){
      ratios.clear();
    }
  
    win_ratio ratie_0{
      .x_ratio = 0,
      .y_ratio = 0,
      .h_ratio = float(getmaxy(this->border[0])) / float(getmaxy(stdscr)),
      .l_ratio = float(getmaxx(this->border[0])) / float(getmaxx(stdscr))
    };
    log_history(logs = to_string(ratie_0.x_ratio) + " " + to_string(ratie_0.y_ratio)+ "\n");
    this->ratios.push_back(ratie_0);

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
    this->total_chars.push_back(0);
    this->total_lines.push_back(0);
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
      this->current_pos.push_back(return_current_line(current_window));
      this->current_pos[current_window-1] = return_current_line(current_window-1);

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
    string err;
    int start = 0;

    string new_line;
    vector<string> lines;
    total_lines[position] = 0;
    for(const auto& x:Window_History[position]){
      splitup_MSG_Restore(lines, new_line, x, position);
      if((int)lines.size() > MAX_HISTORY){
        lines.erase(lines.begin());
      }
    }
    if(!new_line.empty()){
      lines.push_back(new_line);
      new_line.clear();
    }
    total_lines[position] = (int)lines.size();

    Window_History[position].clear();
    for(const auto& l:lines){
      Window_History[position].push_back(l);
    }

    if((int)Window_History[position].size()>(int)getmaxy(inside_box[position])){
      start = (int)Window_History[position].size()-(int)getmaxy(inside_box[position]);
    }
    log_history(err="start = " + to_string(start) + "\n");
    wmove(inside_box[position], 0, 0);

    for(; start < (int)Window_History[position].size(); start++) {
      wprintw(inside_box[position], "%s", Window_History[position][start].c_str());
    }
    err.clear();
    wrefresh(border[position]);
    wrefresh(inside_box[position]);
}



void my_windows::CreateWindowHistory(int position, string &msg)
{
        string toerrishuman;
        log_history(toerrishuman="Clientul a primit mesajul" + msg + "\n");

        vector<string> lines;

        splitup_MSG(lines, msg, position);

        for(const auto& x: lines){
        this->Window_History[position].push_back(x);
        this->total_lines[position]++;

        if(total_lines[position] > MAX_HISTORY){
          total_lines[position]--;
          Window_History[position].erase(Window_History[position].begin());
        }

        }

        this->total_chars[position] += (int)msg.size();
        msg.clear();
        toerrishuman.clear();
}

void my_windows::scrollup(int window)
{
  if(current_pos[window]==0){
    return;
  }
  else{
  string err;
  int start = 0;
  werase(inside_box[window]);
  wclear(inside_box[window]);
  touchwin(inside_box[window]);
  wrefresh(inside_box[window]);
  wmove(inside_box[window], 0, 0);
  if(current_pos[window]>0){
    current_pos[window]--;
  }
  string position = "current pos = " + to_string(current_pos[window]) + "\n";
  log_history(position);
  if(current_pos[window]>getmaxy(inside_box[window])){
    start = Window_History[window].size()-getmaxy(inside_box[window]);
  }
  else if ((int)Window_History[window].size() - current_pos[window] > getmaxy(inside_box[window]))
  {
    start = current_pos[window];
  }

  position.clear();
  position = "current start = " + to_string(start) + "\n";
  log_history(position);

  for(int i=0;i<getmaxy(inside_box[window]);i++){
    if(start+i>(int)Window_History[window].size()){
      break;
    }
    if(start+i==current_pos[window]){
      wattron(inside_box[window], A_REVERSE);
      wprintw(inside_box[window], "%s", Window_History[window][start+i].c_str());
      wattroff(inside_box[window], A_REVERSE);
    }
    else{
      wprintw(inside_box[window], "%s", Window_History[window][start+i].c_str());
    }
  }
}
  

}

void my_windows::scrolldown(int window)
{

  if(current_pos[window]==(int)Window_History[window].size()){
    return;
  }
  else{
  string err;
  int start = 0;
  werase(inside_box[window]);
  wclear(inside_box[window]);
  touchwin(inside_box[window]);
  wrefresh(inside_box[window]);
  wmove(inside_box[window], 0, 0);
  if(current_pos[window]<(int)Window_History[window].size()){
    current_pos[window]++;
  }
  string position = "current pos = " + to_string(current_pos[window]) + "\n";
  log_history(position);
  if(current_pos[window]>getmaxy(inside_box[window])){
    start = Window_History[window].size()-getmaxy(inside_box[window]);
  }
  else if ((int)Window_History[window].size() - current_pos[window] > getmaxy(inside_box[window]))
  {
    start = current_pos[window];
  }

  position.clear();
  position = "current start = " + to_string(start) + "\n";
  log_history(position);

  for(int i=0;i<getmaxy(inside_box[window]);i++){
    if(start+i>(int)Window_History[window].size()){
      break;
    }
    if(start+i==current_pos[window]){
      wattron(inside_box[window], A_REVERSE);
      wprintw(inside_box[window], "%s", Window_History[window][start+i].c_str());
      wattroff(inside_box[window], A_REVERSE);
    }
    else{
      wprintw(inside_box[window], "%s", Window_History[window][start+i].c_str());
    }
  }
}
  

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
  cbreak();   
  //nonl();   
  noecho();
  clear();
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
  int prev_curr_window = current_window;

  windows_opened = 0;
  current_window = -1;

  int start_x = 0;
  int start_y = 0;
  this->enable_log_history = false;
  //log_history(rsz="INcepem\n");
  for(int i=0;i<prev_wincount;i++){
    //log_history(rsz="Am putut face redimensionarea\n");
    current_window = i;
    //log_history(rsz="shit\n");
    //log_history(rsz="ratii:\n" + to_string(this->ratios[i].x_ratio) + " " + to_string(this->ratios[i].y_ratio)+ "\n");
    start_x = getmaxx(stdscr) * this->ratios[i].x_ratio;
    start_y = getmaxy(stdscr) * this->ratios[i].y_ratio;
   // log_history(rsz="DIMENSIUNI:\n" + to_string(getmaxy(stdscr) * this->ratios[i].h_ratio) + " " + to_string(getmaxx(stdscr) * this->ratios[i].l_ratio)+ "\n");
    //log_history(rsz="START POINT:\n" + to_string(start_y) + " " + to_string(start_x)+ "\n");

    Create_Window(getmaxy(stdscr) * this->ratios[i].h_ratio, getmaxx(stdscr) * this->ratios[i].l_ratio, start_y, start_x);
    RestoreWindow(i);
    this->current_pos[i] = return_current_line(i);
    this->enable_log_history = true;
    log_history(rsz = "ultima linie: " + to_string(current_pos[i]) + "\n");
    log_history(rsz = "Numarul total de caractere la fereastra:" + to_string(i)  + "este " + to_string(total_chars[i]) + "\n");
    log_history(rsz = "Numarul total de linii curent:" + to_string(total_lines[i]) + "\n");
  }

  if(prev_curr_window>=0 && prev_curr_window<windows_opened){
    current_window = prev_curr_window;
  }
  else{
    current_window = 0;
  }

  box(border[current_window], 0, 0);
  wrefresh(border[current_window]);
  wrefresh(inside_box[current_window]);
  touchwin(inside_box[current_window]);
  wmove(inside_box[current_window], getcury(inside_box[current_window]), getcurx(inside_box[current_window]));
  doupdate();
  
  refresh();
  this->enable_log_history = true;
  //log_history(rsz);

}

