/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
*/
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
/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;


typedef struct Protocol_Client
{
  void(*procesare_ferestre_paralele)(vector<WINDOW*>& active_windows, vector<PANEL*>& active_panels);
  void(*procesare_ferestre_single)(vector<WINDOW*>& active_windows);
  int(*procesare_cl)(int arc, char* argv[], int &status, WINDOW* wind, int current_window);
  void(*istoric_ferestre)(int chosen_window, string& msg);
  void(*curatenie_de_primavara)(void);
}Protocol_Client;






// void my_print(WINDOW* win, int &y_coord, int &x_coord, const char* msg){
//     mvwprintw(win, y_coord, x_coord, "%s", msg);
//     y_coord++;
//     wrefresh(win);
// }


void log_history(string &msg);
int procesare_client(int argc, char *argv[], int &connected, WINDOW* win, int current_window);
WINDOW* create_window (int height, int width, int y, int x);
void process_windows(vector<WINDOW*>& active_windows, vector<PANEL*>& active_panels);
void process_singlewindow(vector<WINDOW*>& active_windows);
void window_history(int chosen_window, string& msg);
void restore_window(WINDOW* win, int window_number);
void cleanup();

Protocol_Client My_Client{
  .procesare_ferestre_paralele = process_windows,
  .procesare_ferestre_single = process_singlewindow,
  .procesare_cl = procesare_client,
  .istoric_ferestre = window_history,
  .curatenie_de_primavara = cleanup
};

int main (int argc, char *argv[])
{ 
 // int x_coord=0, y_coord=0;
  int connected = 0; 

  initscr();
  cbreak();
  remove("my_logs");
  vector<WINDOW*> ferestre_active;
  vector<PANEL*> panouri_active;
  string log_updates;
  char key='i';
  My_Client.curatenie_de_primavara();



  //int current_option = 0;
  //int max_options = 2;
  //int do_loop = 1;  
  //vector<string> options = {"SIngle Window", "Multi Window"};
  //main menu

  // while(do_loop){
  // mvprintw(getmaxy(stdscr)/2, getmaxx(stdscr)/2, "Bine ati venit pe programul meu! Va rugam selectati modul de operare preferat\n");
  //   switch (key)
  //   {
  //     case 10:
  //     do_loop = 0;
  //     break;
  //     case '\t':
  //     if(current_option+1>=max_options){
  //       current_option = 0;
  //     }
  //     else{
  //       current_option++;
  //     }

  //   }
  // }


//   process_windows(ferestre_active, panouri_active);
//   int current_window = 0;
//   while(key != 'q'){
//     //log_history(log_updates = "Am intrat in loop\n");
//     //log_history(log_updates = "Acum alegem alta cheie...\n");
//     key = wgetch(ferestre_active[current_window]);
//     switch (key)
//     {
//     case 10:

//     log_history(log_updates = "Clintul tasteaza\n");

//     if(My_Client.procesare_cl(argc, argv, connected, ferestre_active[current_window])!=1){
//       log_history(log_updates = "A avut loc o eroare!\n");
//       break;
//      };
//     wprintw(ferestre_active[current_window],"[Client] Do you want continue? y/n\n");
//     wrefresh(ferestre_active[current_window]);
//     while((key!='y') && (key!='Y') && (key!='n') && (key!='N')){
//       noecho();
//       key = wgetch(ferestre_active[current_window]);
//       echo();
//     }
//     if((key=='n') || (key=='N')){
//       log_history(log_updates = "Am ales sa nu mai continui!\n");
//       break;
//     }
    
//       break;
//     case '\t':
//     log_history(log_updates = "Schimbam fereastra!\n");
//       if(current_window>(int)panouri_active.size()-2){
//         current_window = 0;
//       }
//       else{
//         current_window++;
//       }
//       top_panel(panouri_active[current_window]);
//       wrefresh(ferestre_active[current_window]);
//       break;
//     case '=':
//       process_windows(ferestre_active, panouri_active);
//       current_window = ferestre_active.size()-1;
//       log_history(log_updates = "Am creat o fereastra noua, cu numarul " + to_string((int)ferestre_active.size()) + "\n");
//       break;
    
//     default:
//       break;
//     }
// }

My_Client.procesare_ferestre_single(ferestre_active);
int current_window = 0;
string msg;
while(key != 'q'){
  //log_history(log_updates = "Am intrat in loop\n");
  //log_history(log_updates = "Acum alegem alta cheie...\n");
  noecho();
  key = wgetch(ferestre_active[current_window]);
  switch (key)
  {
  case 10:
  echo();
  while(true){
  log_history(log_updates = "Clientul tasteaza\n");
  if(My_Client.procesare_cl(argc, argv, connected, ferestre_active[current_window], current_window)!=1){
    log_history(log_updates = "A avut loc o eroare!\n");
    break;
   };
  wprintw(ferestre_active[current_window],"[Client] Do you want continue? y/n\n");
  My_Client.istoric_ferestre(current_window, msg = "[Client] Do you want continue? y/n\n");
  wrefresh(ferestre_active[current_window]);
  while((key!='y') && (key!='Y') && (key!='n') && (key!='N')){
    noecho();
    key = wgetch(ferestre_active[current_window]);
    echo();
  }
  if((key=='n') || (key=='N')){
    log_history(log_updates = "Am ales sa nu mai continui!\n");
    break;
  }
  else{
    key='i';
  }
  } 
  break;


  case '\t':
  log_history(log_updates = "Schimbam fereastra!\n");
    if(current_window>(int)ferestre_active.size()-2){
      current_window = 0;
    }
    else{
      current_window++;
    }
    wrefresh(ferestre_active[current_window]);
    break;
  case '=':
    My_Client.procesare_ferestre_single(ferestre_active);
    current_window = ferestre_active.size()-1;
    log_history(log_updates = "Am creat o fereastra noua, cu numarul " + to_string((int)ferestre_active.size()) + "\n");
    break;
  
  default:
    break;
  }
}
  My_Client.curatenie_de_primavara();
  endwin();
}

void log_history(string &msg){
  int fd = open("./my_logs", O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
    perror("EROARE LA DESCHIDEREA FISIERULUI!");
    return;
  }
  if(write(fd, msg.c_str(), msg.size())<=-1){
    return;
  }
  msg.clear();
  close(fd);
}

int procesare_client(int argc, char *argv[], int &connected, WINDOW* win, int current_window){
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  int x_coord, y_coord;
  int nr=0;
  string msg;
  char buf[4096];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      string argv_zero;
      argv_zero.assign(argv[0], argv[0] + strlen(argv[0]));
      msg = "Sintaxa: " + argv_zero  + "<adresa_server> <port>\n";
      argv_zero.clear();
      My_Client.istoric_ferestre(current_window, msg);
      wprintw(win, "%s", msg.c_str());
      wgetch(win);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      msg = "Eroare la socket().\n";
      My_Client.istoric_ferestre(current_window, msg);
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);

  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      wprintw(win, "[client]Eroare la connect().\n");
      refresh();
      int i;
      for(i=5;i>=0;i--){
        getyx(win, y_coord, x_coord);
        mvwprintw (win, y_coord, x_coord, "Window will automatically close in %d seconds...\n", i);
        refresh();
        sleep(1);
        }
        msg = "Window will automatically close in" + to_string(i) + "seconds...\n";
        My_Client.istoric_ferestre(current_window, msg);

      perror("");
      
      return errno;
    }

  /* citirea mesajului */

  keypad(stdscr, TRUE);
  if(connected==0){
    msg = "[Server] V-ati connectat cu succes!\n";
    My_Client.istoric_ferestre(current_window, msg);
    wprintw(win, "[Server] V-ati connectat cu succes!\n");
    wrefresh(win);
    connected = 1;
  }


  wprintw(win, "[client]Introduceti o comanda: ");
  msg = "[client]Introduceti o comanda: ";
  My_Client.istoric_ferestre(current_window, msg);
  wrefresh(win);
  wgetnstr(win, buf, 4096);
  msg = buf;
  msg = msg + "\n";
  My_Client.istoric_ferestre(current_window, msg);
  
  nr = strlen(buf)+1;
  if (write (sd,&nr,sizeof(int)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      log_history(msg = "[client]Eroare la write() spre server.\n");
      return errno;
    }

  if (write (sd, buf, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      log_history(msg = "[client]Eroare la write() spre server.\n");
      return errno;
    }


  if (read (sd, &nr,sizeof(int)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      log_history(msg = "[client]Eroare la read() de la server.\n");
      return errno;
    }
  else{
    msg = "[Server]" + to_string(nr) + "biti\n";
    log_history(msg);
  }

  char raspuns[nr+1];

  if (read (sd, raspuns,nr) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      log_history(msg = "[client]Eroare la read() de la server.\n");
      return errno;
    }
  raspuns[nr] = '\0';

  /* afisam mesajul primit */
  wprintw (win,"[Server]%s\n", raspuns);
  string temp = raspuns;
  msg = "[Server] " + temp + " \n";
  temp.clear();
  My_Client.istoric_ferestre(current_window, msg);
  log_history(msg);
  wrefresh(win);

  /* inchidem conexiunea, am terminat */
  close (sd);
  return 1;
}

WINDOW* create_window (int height, int width, int y, int x){
  WINDOW* new_window = newwin(height, width, y, x);
  box(new_window, 0, 0);
  //WINDOW* innernew_window = newwin(height-1, width-1, y+1, x+1);
  wrefresh(new_window);
  //wrefresh(innernew_window);
  return new_window;
}

void process_windows(vector<WINDOW*>& active_windows, vector<PANEL*>& active_panels){
  if(active_windows.size()!=10){
  WINDOW* new_window = create_window(getmaxy(stdscr), getmaxx(stdscr), 0, 0);
  active_windows.push_back(new_window);
  idlok(new_window, true);
  //wbkgd(new_window, COLOR_PAIR(1));
  keypad(new_window, true);
  mvwprintw(new_window, getmaxy(new_window)-1, getmaxx(new_window)/2, "Page %d", (int)active_windows.size());
  wmove(new_window, 1, 1);
  PANEL* new_pan = new_panel(new_window);
  active_panels.push_back(new_pan);
  update_panels();
  doupdate();
  }
}

void process_singlewindow(vector<WINDOW*>& active_windows){
  int maxx_win, maxy_win, init_x, init_y;
  WINDOW* new_window;
  string logs;
  if(active_windows.size()==0){

    maxx_win = getmaxx(stdscr);
    maxy_win = getmaxy(stdscr);
    init_x = getbegx(stdscr);
    init_y = getbegy(stdscr);
    new_window = create_window(maxy_win, maxx_win, init_y, init_x);
    active_windows.push_back(new_window);
    idlok(new_window, true);
    scrollok(new_window, true);
    //wbkgd(new_window, COLOR_PAIR(1));
    keypad(new_window, true);
    mvwprintw(new_window, getmaxy(new_window)-1, getmaxx(new_window)/2, "%d", (int)active_windows.size());
    wmove(new_window, 1, 1);
    wprintw(new_window, "Press Enter to start typing, + to create a new window or tab to switch between windows!");
    logs =  "Press Enter to start typing, + to create a new window or tab to switch between windows!";
    My_Client.istoric_ferestre((int)active_windows.size()-1, logs);
    wrefresh(new_window);
    log_history(logs = "Avem " + to_string((int)active_windows.size()) + " ferestre active!\n");
  }
  else if(active_windows.size()!=6){
    
    log_history(logs = "Avem " + to_string((int)active_windows.size()) + " ferestre active!\n");
    maxx_win = getmaxx(active_windows[active_windows.size()-1]);
    maxy_win = getmaxy(active_windows[active_windows.size()-1]);
    init_x = getbegx(active_windows[active_windows.size()-1]);
    init_y = getbegy(active_windows[active_windows.size()-1]);
    
    if(active_windows.size()%2==0){
    new_window = create_window(maxy_win/2, maxx_win, init_y+maxy_win/2, init_x);
    }
    else{
      new_window = create_window(maxy_win, maxx_win/2, init_y, init_x+maxx_win/2);
    }
    active_windows.push_back(new_window);
    
    idlok(new_window, true);
    scrollok(new_window, true);
    //wbkgd(new_window, COLOR_PAIR(1));
    keypad(new_window, true);
    //mvwprintw(new_window, getmaxy(new_window)-1, getmaxx(new_window)/2, "%d", (int)active_windows.size());
    wmove(new_window, 1, 1);
    My_Client.istoric_ferestre((int)active_windows.size()-1, logs="");


    if((active_windows.size()-2)%2==0){
    // wborder(active_windows[active_windows.size()-2], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(active_windows[active_windows.size()-2]);
    wclear(active_windows[active_windows.size()-2]);
    touchwin((active_windows[active_windows.size()-2]));
    wrefresh(active_windows[active_windows.size()-2]);
    wresize(active_windows[active_windows.size()-2], maxy_win, maxx_win/2);
    wmove(active_windows[active_windows.size()-2], 1, 1);
    restore_window(active_windows[active_windows.size()-2], active_windows.size()-2);
    //mvwprintw(active_windows[active_windows.size()-2], getmaxy(active_windows[active_windows.size()-2])-1, getmaxx(active_windows[active_windows.size()-2])/2, "%d", (int)active_windows.size()-1);
    //wmove(new_window, 1, 1);
    }
    else{
     // wborder(active_windows[active_windows.size()-2], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
      werase(active_windows[active_windows.size()-2]);
      wclear(active_windows[active_windows.size()-2]);
      touchwin((active_windows[active_windows.size()-2]));
      wrefresh(active_windows[active_windows.size()-2]);
      wmove(active_windows[active_windows.size()-2], 1, 1);
      wresize(active_windows[active_windows.size()-2], maxy_win/2, maxx_win);
      restore_window(active_windows[active_windows.size()-2], active_windows.size()-2);
      //mvwprintw(active_windows[active_windows.size()-2], getmaxy(active_windows[active_windows.size()-2])-1, getmaxx(active_windows[active_windows.size()-2])/2, "%d", (int)active_windows.size()-1);
      //wmove(new_window, 1, 1);
     
    }
    box(new_window, 0, 0);
    box(active_windows[active_windows.size()-2], 0, 0);
    wrefresh(active_windows[active_windows.size()-2]);
  }

}


void window_history(int chosen_window, string& msg){
  string nume_fisier = "./Client_logs/window" + to_string(chosen_window);
  string toerrishuman;
  int fd = open(nume_fisier.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
    perror("ERORARE LA CITIREA/CREEREA FISIERULUI!");
    log_history(toerrishuman = "A avut loc o eroare la creerea fisierului window" + to_string(chosen_window)+"\n");
    return;
  }
  if(write(fd, msg.c_str(), msg.size())==-1){
    perror("EROARE LA SCRIEREA PE FISIER!");
    log_history(toerrishuman = "A avut loc o eroare la scrierea pe fisierul window" + to_string(chosen_window)+"\n");
    return;
  }
  msg.clear();
  nume_fisier.clear();
  toerrishuman.clear();
  close(fd);
}

void restore_window(WINDOW* win, int window_number){
  string istoric_fereastra = "./Client_logs/window" + to_string(window_number);
  string err;
  struct stat stbuf;
  int fd = open(istoric_fereastra.c_str(), O_RDONLY);
  if(fd==-1){
    perror("ERORARE LA DESCHIDEREA FISIERULUI!");
    log_history(err= "A avut loc o eroare la deschiderea fisierului window" + to_string(window_number)+"\n");
    return;
  }
  if(lstat(istoric_fereastra.c_str(), &stbuf)==-1){
    perror("EROARE LA lstat!");
    log_history(err = "EROARE LA lstat!");
    return;
  }

  int file_size = stbuf.st_size;
  char istory[file_size+1];

  if(read(fd, istory, file_size)==-1){
    perror("EROARE LA CITIRE DIN FISIER!");
    log_history(err = "EROARE LA CITIREA DIN FISIERUL window"+to_string(window_number)+"\n");
    return;
  }
  istory[file_size] = '\0';
  wprintw(win, "%s", istory);
  istoric_fereastra.clear();
  err.clear();
  close(fd);
  wrefresh(win);
}

void cleanup(){
  string file_history;
  for(int i=0;i<=10;i++){
    file_history = "./Client_logs/window" + to_string(i);
    remove(file_history.c_str());
  }
}