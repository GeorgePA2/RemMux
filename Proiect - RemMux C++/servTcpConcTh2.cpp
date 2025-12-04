
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <signal.h>
#include <vector>
#include <string>
#include <pthread.h>
#include <fcntl.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

typedef struct Protocol
{
  int(*valideaza_comanda)(char* comanda);
  char*(*tipul_operatiei)(const char* comanda);
  void(*executa)(const char* comanda, void* arg);
  void(*returneaza)(void* arg, int code);
}Protocol;


static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int find_sep(const char* string_seq, const char* sep){
  
  int i = 0;
  int k = 0;

  while(i<strlen(string_seq)){
    printf("c: %c\n", string_seq[i]);
    if(string_seq[i]==sep[k]){
    printf("realizam compararea intre %c si %c \n", string_seq[i], sep[k]);
    printf("string_seq[%d+%d+1] si sep[%d+1], adica %c si %c\n", i, k, k, string_seq[i+k+1], sep[k+1]);
      while(string_seq[i+k+1]==sep[k+1]){
      	printf("string_seq[%d+%d+1]==sep[%d+1], adica %c = %c\n", i, k, k, string_seq[i+k+1], sep[k+1]);
        k++;
        printf("%d\n", k);
      }
      printf("k este %d si lungimea sep este %d\n", k, strlen(sep));
      if(k==strlen(sep)-1){
      	// char* result = new char[strlen(sep)+1];
        // strcpy(result, sep);
        // result[strlen(sep)]= '\0';
        // strncpy(result, string_seq, i);
        // result[i] = '\0';
        // printf("%s\n", result);
        // return result;
        return 1;
      }
      else{
         i = i+k;
         k=0;
      }
    }
    i++;
  }
  // char* result = new char[i+1];
  // strncpy(result, string_seq, i);
  // result[i] = '\0';
  return 0;
}


int comanda_valida(char* cmd);
char* op_type(const char* cmd);
void exec_cmd(const char* cmd, void* arg);
void return2cl(void* arg, int cod);

using namespace std;


Protocol MyProtocol{
  .valideaza_comanda = comanda_valida,
  .tipul_operatiei = op_type,
  .executa = exec_cmd,
  .returneaza = return2cl
};

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      socklen_t length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
    free(arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
  int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}
	
	printf ("[Thread %d]Marimea mesajului a fost receptionata...%d\n",tdL.idThread, nr);
  if((nr<=-1) || (nr>10000)){
    printf("Something unexpected happened...");
    MyProtocol.returneaza(arg, 2);
  }
  char comanda_primita[nr+1];
  if(read(tdL.cl, comanda_primita, sizeof(comanda_primita))<=0){
    printf("[Thread %d]\n",tdL.idThread);
    perror ("Eroare la read() de la client.\n");
  }
  printf ("[Thread %d]Comanda a fost receptionata...%s\n",tdL.idThread, comanda_primita);

  printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);
  if(MyProtocol.valideaza_comanda(comanda_primita)==1){
    printf("Comanda a fost validata! \n");
    MyProtocol.executa(comanda_primita, arg);
    printf("Comanda a fost executata! \n");
    MyProtocol.returneaza(arg, 0);
  }
  else{
    printf("Comanda NU a fost validata! \n");
    MyProtocol.returneaza(arg, 1);
  }


		    

}

int comanda_valida(char* cmd){
  char* cmd_temp = new char[strlen(cmd)+1];
  strcpy(cmd_temp, cmd);
  cmd_temp[strlen(cmd)]='\0';

  char cmd_path[256];
  strcpy(cmd_path, "/usr/bin/");

  if(strtok(cmd_temp, " ")==NULL){
    strcat(cmd_path, cmd);
  }
  else{
    strcat(cmd_path, cmd_temp);
  }

  printf("cmd_path = %s\n", cmd_path);

  int fd = open(cmd_path, O_RDONLY);

  if(fd<-0){
    return 0;
  }
  else{
    close(fd);
    return 1;
  }
}

char* op_type(const char* cmd){
  vector<string> separatori = {" ", " && ", " || ", " | ", "2>", "<", ">"};
  for(const auto& x : separatori){
    if(find_sep(cmd, x.c_str())==1){
      char* result = new char[strlen(x.c_str())+1];
      strcpy(result, x.c_str());
      result[strlen(x.c_str())] = '\0';
      return result;
    }
  }
  return "N/A";
}

void exec_cmd(const char* cmd, void* arg){
  struct thData tdL; 
	tdL= *((struct thData*)arg);
  string filename = "./Files/temp" + to_string(tdL.idThread);
  int file_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(file_fd==-1){
    perror("EROARE LA DESCHIDEREA/CREEREA FILEI!");
  }
  int nr = strlen(cmd);
  if((write(file_fd, &nr, sizeof(int)))==-1){
    perror("EROARE LA TRIMITEREA BITILOR!");
  }
  if((write(file_fd, cmd, strlen(cmd)))<=-1){
    perror("WTF");
  }
  //sleep(3);
  close(file_fd);
}


void return2cl(void* arg, int is_err){
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  string filename = "./Files/temp" + to_string(tdL.idThread);
  int sizeofmsg;

  if(is_err==1){
    char mesaj_de_eroare[] = "Eroare! Comanda inexistenta!";
    sizeofmsg = strlen(mesaj_de_eroare)-1;
    
    if(write(tdL.cl, &sizeofmsg, sizeof(int))<=0){
      printf("[Thread %d] ",tdL.idThread);
      perror ("[Thread]Eroare la write() catre client.\n");
    }
    else{
      printf ("[Thread %d]Numarul de biti %d a fost trasmis cu succes.\n",tdL.idThread, sizeofmsg);	
    }
    if(write(tdL.cl, mesaj_de_eroare, sizeofmsg)<=0){
      printf("[Thread %d] ",tdL.idThread);
      perror ("[Thread]Eroare la write() catre client.\n");
    }
    else{
      printf ("[Thread %d]Mesajul : %s de eroare transmis cu succes!.\n",tdL.idThread, mesaj_de_eroare);	
    }
  }

  else if(is_err==0){
  int fd = open(filename.c_str(), O_RDONLY);
  if(fd==-1){
    perror("EROARE LA DESCHIDEREA FISIERULUI!");
  }
  if(read(fd, &sizeofmsg, sizeof(int))==-1){
    perror("EROARE LA CITIRE!");
  }
  char msg[sizeofmsg];
  if(read(fd, msg, sizeof(msg))==-1){
    perror("EROARE LA CITIREA MESAJULUI!");
  }
	 if (write (tdL.cl, &sizeofmsg, sizeof(int)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else{
		printf ("[Thread %d]Numarul de biti %d a fost trasmis cu succes.\n",tdL.idThread, sizeofmsg);	
    msg[sizeofmsg] = '\0';
     if (write (tdL.cl, msg, sizeofmsg) <= 0)
      {
       printf("[Thread %d] ",tdL.idThread);
       perror ("[Thread]Eroare la write() catre client.\n");
      }
    else{
      printf ("[Thread %d]Mesajul %s a fost transmis cu success.\n",tdL.idThread, msg);	
      close(fd);
    }
    }
  }
  
  else if(is_err==2){
      char mesaj_de_eroare[] = "Mesajul tau nu poate fi acceptat de catre acest server!";
      sizeofmsg = strlen(mesaj_de_eroare)-1;
      
      if(write(tdL.cl, &sizeofmsg, sizeof(int))<=0){
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
      }
      else{
        printf ("[Thread %d]Numarul de biti %d a fost trasmis cu succes.\n",tdL.idThread, sizeofmsg);	
      }
      if(write(tdL.cl, mesaj_de_eroare, sizeofmsg)<=0){
        printf("[Thread %d] ",tdL.idThread);
        perror ("[Thread]Eroare la write() catre client.\n");
      }
      else{
        printf ("[Thread %d]Mesajul : %s de eroare transmis cu succes!.\n",tdL.idThread, mesaj_de_eroare);	
      }
    }
  

  remove(filename.c_str());
}
