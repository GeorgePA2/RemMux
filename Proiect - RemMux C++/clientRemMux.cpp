/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@info.uaic.ro> (c)
*/
#include "./My_Classes/my_windows.h"
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
#include <signal.h>
#include <panel.h>


using namespace std;

extern int errno;


int port;


typedef struct Protocol_Client
{

  int(*procesare_cl)(int arc, char* argv[], int &status, my_windows& wind);

  void(*curatenie_de_primavara)(void);
}Protocol_Client;










void log_history(string &msg);
 int procesare_client(int argc, char *argv[], int &connected, my_windows& win);

 void cleanup();

Protocol_Client My_Client{

  .procesare_cl = procesare_client,

  .curatenie_de_primavara = cleanup
};

int main (int argc, char *argv[])
{ 

  int connected = 0; 
  initscr();
  cbreak();
  remove("my_logs");
  string log_updates;
  int key='i';
  My_Client.curatenie_de_primavara();
  my_windows active_windows;
  string msg;
  while((key != 'q') && (key !='Q')){

  noecho();
  key = wgetch(active_windows.Get_CurrentWindow());


  switch (key){

    case 10:
      echo();
      while(true){
        log_history(log_updates = "Clientul tasteaza\n");
        if(My_Client.procesare_cl(argc, argv, connected, active_windows)!=1){
            wprintw(active_windows.Get_CurrentWindow(), "[Client]A avut loc o eroare in server...\n");
            active_windows.CreateWindowHistory(active_windows.Get_CurrentW(), msg = "[Client]A avut loc o eroare in server...\n");
            log_history(log_updates = "A avut loc o eroare!\n");
            connected = 0;
            break;
          };
        wprintw(active_windows.Get_CurrentWindow(),"[Client] Do you want continue? y/n\n");
        active_windows.CreateWindowHistory(active_windows.Get_CurrentW(), msg = "[Client] Do you want continue? y/n\n");
        wrefresh(active_windows.Get_CurrentWindow());
        while((key!='y') && (key!='Y') && (key!='n') && (key!='N')){
          
          if (key==KEY_RESIZE){
            active_windows.resize_win();
            log_history(log_updates="resizing...\n");
           }

          noecho();
          key = wgetch(active_windows.Get_CurrentWindow());
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
  log_updates = "Schimbam fereastra, de la" + to_string(active_windows.Get_CurrentW()) + " la ";
    if(active_windows.Get_CurrentW()>active_windows.Get_nrWindows()-2){
      active_windows.Change_CurrWin(0);
      
    }
    else{
      active_windows.Change_CurrWin(active_windows.Get_CurrentW()+1);
    }
    log_updates = log_updates + to_string(active_windows.Get_CurrentW()) + "\n";
    log_history(log_updates);
    wrefresh(active_windows.Get_CurrentWindow());
    break;
  case '=':
    active_windows.Add_Window();
    log_history(log_updates = "Am creat o fereastra noua, cu numarul " + to_string(active_windows.Get_nrWindows()) + "\n");
    break;

  case KEY_RESIZE:
    active_windows.resize_win();
    log_history(log_updates="resizing...\n");
    break;

  case -1:
    log_history(log_updates="RAGE QUIT\n");
    key = 'q';
    break;

  default:
    log_history(log_updates="Comanda nerecunoscuta" + to_string(key) + "\n");
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

int procesare_client(int argc, char *argv[], int &connected, my_windows& win){
  int sd;			
  struct sockaddr_in server;	
  int x_coord, y_coord;
  int nr=0;
  string msg;
  char buf[4096];

  if (argc != 3)
    {
      string argv_zero;
      argv_zero.assign(argv[0], argv[0] + strlen(argv[0]));
      msg = "Sintaxa: " + argv_zero  + "<adresa_server> <port>\n";
      argv_zero.clear();
      win.CreateWindowHistory(win.Get_CurrentW(), msg);
      wprintw(win.Get_CurrentWindow(), "%s", msg.c_str());
      wgetch(win.Get_CurrentWindow());
      return -1;
    }


  port = atoi (argv[2]);

  
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      msg = "Eroare la socket().\n";
      win.CreateWindowHistory(win.Get_CurrentW(), msg);
     
      return -1;
    }


  server.sin_family = AF_INET;

  server.sin_addr.s_addr = inet_addr(argv[1]);

  server.sin_port = htons (port);

  
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      wprintw(win.Get_CurrentWindow(), "[client]Eroare la connect().\n");
      win.CreateWindowHistory(win.Get_CurrentW(), msg = "[client]Eroare la connect().\n");
      wrefresh(win.Get_CurrentWindow());
      int i;
      getyx(win.Get_CurrentWindow(), y_coord, x_coord);
      for(i=5;i>=0;i--){
        mvwprintw (win.Get_CurrentWindow(), y_coord, x_coord, "Veti fi trimis inapoi catre meniu in %d secunde...\n", i);
        wrefresh(win.Get_CurrentWindow());
        sleep(1);
        }
        msg = "Veti fi trimis inapoi catre meniu in" + to_string(0) + "secunde...\n";
        win.CreateWindowHistory(win.Get_CurrentW(), msg);

     
      close(sd);
      
      return -1;
    }

  

  if(connected==0){
    msg = "[Server] V-ati connectat cu succes!\n";
    win.CreateWindowHistory(win.Get_CurrentW(), msg);
    wprintw(win.Get_CurrentWindow(), "[Server] V-ati connectat cu succes!\n");
    wrefresh(win.Get_CurrentWindow());
    connected = 1;
  }


  wprintw(win.Get_CurrentWindow(), "[client]Introduceti o comanda: ");
  msg = "[client]Introduceti o comanda: ";
  win.CreateWindowHistory(win.Get_CurrentW(), msg);
  wrefresh(win.Get_CurrentWindow());
  wgetnstr(win.Get_CurrentWindow(), buf, 4096);
  msg = buf;
  msg = msg + "\n";
  
  win.CreateWindowHistory(win.Get_CurrentW(), msg);
  nr = strlen(buf)+1;
  signal(SIGPIPE, SIG_IGN);
  if (write (sd,&nr,sizeof(int)) <= 0)
    {
     
      log_history(msg = "[client]Eroare la write() spre server.\n");
      return -1;
    }

  if (write (sd, buf, nr) <= 0)
    {
     
      log_history(msg = "[client]Eroare la write() spre server.\n");
      return -1;
    }


  if (read (sd, &nr,sizeof(int)) < 0)
    {
     
      log_history(msg = "[client]Eroare la read() de la server.\n");
      return -1;
    }
  else{
    msg = "[Server]" + to_string(nr) + "biti\n";
    log_history(msg);
  }

  char raspuns[nr+1];
  signal(SIGPIPE, SIG_IGN);
  if (read (sd, raspuns,nr) < 0)
    {
     
      log_history(msg = "[client]Eroare la read() de la server.\n");
      return -1;
    }
  raspuns[nr] = '\0';
  signal(SIGPIPE, SIG_IGN);

  
  wprintw (win.Get_CurrentWindow(),"[Server]%s\n", raspuns);
  string temp = raspuns;
  msg = "[Server] " + temp + " \n";
  temp.clear();
  win.CreateWindowHistory(win.Get_CurrentW(), msg);
  log_history(msg);
  wrefresh(win.Get_CurrentWindow());

  
  close (sd);
  return 1;
}

void cleanup(){
  string file_history;
  for(int i=0;i<=10;i++){
    file_history = "./Client_logs/window" + to_string(i);
    remove(file_history.c_str());
  }
}