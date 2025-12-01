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

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int procesare_client(int argc, char *argv[], int &x_coord, int &y_coord, int &connected, WINDOW* win);
WINDOW* create_window (int height, int width, int y, int x);
void my_print(WINDOW* win, int &y_coord, int &x_coord, const char* msg){
    mvwprintw(win, y_coord, x_coord, "%s", msg);
    y_coord++;
    wrefresh(win);
}

using namespace std;
int main (int argc, char *argv[])
{ 
  int x_coord=0, y_coord=0;
  int connected = 0; 

  initscr();
  cbreak();
  
  char key='i';  
  while(key != KEY_F(1)){
  if(procesare_client(argc, argv, x_coord, y_coord, connected, stdscr)!=1){
    break;
  };
  refresh();
  key = getch();
  while((key!='y') && (key!='Y') && (key!='n') && (key!='N')){
    my_print(stdscr, y_coord, x_coord, "[Client] Do you want continue? y/n\n");
    refresh();
    key = getch();

  }
  if((key=='n') || (key=='N')){
    break;
  }
}
  endwin();
}

int procesare_client(int argc, char *argv[], int &x_coord, int &y_coord, int &connected, WINDOW* win){
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 

  int nr=0;
  char buf[4096];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      mvwprintw(win, y_coord, x_coord, "Sintaxa: %s <adresa_server> <port>\n", argv[0]);
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
      my_print(win, y_coord, x_coord, "[client]Eroare la connect().\n");
      y_coord++;
      refresh();
      for(int i=5;i>=0;i--){
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
  my_print(win,y_coord, x_coord, "[Server] V-ati connectat cu succes!");
  connected = 1;
  }


  my_print(win, y_coord, x_coord, "[client]Introduceti o comanda: ");
  getstr(buf);
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
    mvwprintw (win, y_coord, x_coord,"[Server]%d biti: \n",nr);
    y_coord++;
  }

  char raspuns[nr+1];

  if (read (sd, raspuns,nr) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  raspuns[nr] = '\0';

  /* afisam mesajul primit */
  mvwprintw (win, y_coord, x_coord,"[Server]%s\n", raspuns);
  y_coord++;
  refresh();

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