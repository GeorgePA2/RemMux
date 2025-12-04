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
  int(*procesare_cl)(int arc, char* argv[], int &status, WINDOW* wind);
  void(*istoric_ferestre)(int chosen_window, string msg);
}Protocol_Client;




int procesare_client(int argc, char *argv[], int &connected, WINDOW* win);
WINDOW* create_window (int height, int width, int y, int x);
// void my_print(WINDOW* win, int &y_coord, int &x_coord, const char* msg){
//     mvwprintw(win, y_coord, x_coord, "%s", msg);
//     y_coord++;
//     wrefresh(win);
// }

void log_history(string &msg){
  int fd = open("./my_logs", O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
    perror("EROARE LA DESCHIDEREA FISIERULUI!");
  }
  if(write(fd, msg.c_str(), msg.size())<=-1){
    perror("EROARE LA TRIMITEREA MESAJULUI!!");
  }
  msg.clear();
  close(fd);
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

void process_singlewindow(vector<WINDOW*>& active_windows);

Protocol_Client My_Client{
  .procesare_ferestre_paralele = process_windows,
  .procesare_ferestre_single = process_singlewindow,
  .procesare_cl = procesare_client
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



  //int current_option = 0;
  //int max_options = 2;
  //int do_loop = 1;  
  vector<string> options = {"SIngle Window", "Multi Window"};
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
  log_history(log_updates = "Clintul tasteaza\n");
  if(My_Client.procesare_cl(argc, argv, connected, ferestre_active[current_window])!=1){
    log_history(log_updates = "A avut loc o eroare!\n");
    break;
   };
  wprintw(ferestre_active[current_window],"[Client] Do you want continue? y/n\n");
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
  endwin();
}

int procesare_client(int argc, char *argv[], int &connected, WINDOW* win){
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  int x_coord, y_coord;
  int nr=0;
  char buf[4096];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      wprintw(win, "Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      getch();
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
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
      for(int i=5;i>=0;i--){
        getyx(win, y_coord, x_coord);
        mvwprintw (win, y_coord, x_coord, "Window will automatically close in %d seconds...\n", i);
        refresh();
        sleep(1);
        }

      perror("");
      
      return errno;
    }

  /* citirea mesajului */

  keypad(stdscr, TRUE);
  if(connected==0){
  wprintw(win, "[Server] V-ati connectat cu succes!\n");
  wrefresh(win);
  connected = 1;
  }


  wprintw(win, "[client]Introduceti o comanda: ");
  wrefresh(win);
  wgetstr(win, buf);
  //scanf("%d",&nr);
  
  // mvprintw(y_coord, x_coord, "[client] Am citit %s\n",buf);
  // y_coord++;
  // refresh();
  
  nr = strlen(buf)+1;
  if (write (sd,&nr,sizeof(int)) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  if (write (sd, buf, nr) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }


  if (read (sd, &nr,sizeof(int)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  else{
    wprintw (win, "[Server]%d biti: \n",nr);
  }

  char raspuns[nr+1];

  if (read (sd, raspuns,nr) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  raspuns[nr] = '\0';

  /* afisam mesajul primit */
  wprintw (win,"[Server]%s\n", raspuns);
  wrefresh(win);

  /* inchidem conexiunea, am terminat */
  close (sd);
  return 1;
}

WINDOW* create_window (int height, int width, int y, int x){
  WINDOW* new_window = newwin(height, width, y, x);
  box(new_window, 0, 0);
  wrefresh(new_window);
  return new_window;
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
    mvwprintw(new_window, getmaxy(new_window)-1, getmaxx(new_window)/2, "%d", (int)active_windows.size());
    wmove(new_window, 1, 1);
    if((active_windows.size()-2)%2==0){
    wresize(active_windows[active_windows.size()-2], maxy_win, maxx_win/2);
    }
    else{
      wresize(active_windows[active_windows.size()-2], maxy_win/2, maxx_win);
    }
    box(active_windows[active_windows.size()-2], 0, 0);
    wrefresh(active_windows[active_windows.size()-2]);
  }

}