#include "./My_Classes/Commandments.h"
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
#include <sys/wait.h>
#include <sys/stat.h>
#include <fstream>


#define PORT 2908
using namespace std;

extern int errno;

typedef struct thData{
	int idThread; 
	int cl; 
}thData;



typedef struct Protocol
{
  void(*executa)(const char* comanda, void* arg);
  void(*returneaza)(void* arg, int code);
}Protocol;


static void *treat(void *); 
void raspunde(void *);
int find_sep(const char* string_seq, const char* sep);
int comanda_valida(char* cmd);
void exec_cmd(const char* cmd, void* arg);
void return2cl(void* arg, int cod);
void pregateste_comanda(const char* cmd, char** &vector_de_comenzi);

void executa_mult_cmd(Commandments command, string filename, string filename_temp);
void exec_generic_cmd(Commandments my_command, int nr_cmd, string filename);
void handle_execution(Commandments cmd);
void exec_sg_cmd(Commandments my_command, string filename);


void ORwell(Commandments cmd, int nr_cmd, string filename);
void pipeline(Commandments cmd, int nr_cmd, string filename);
void AND_oftheworld(Commandments cmd, int nr_cmd, string filename);
void DotCom(Commandments cmd, int nr_cmd, string filename);



Protocol MyProtocol{
  .executa = exec_cmd,
  .returneaza = return2cl
};

int main ()
{
  struct sockaddr_in server;	
  struct sockaddr_in from;	
  int sd;		
  pthread_t th[100];   
	int i=0;
  


  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));

    server.sin_family = AF_INET;	

    server.sin_addr.s_addr = htonl (INADDR_ANY);

    server.sin_port = htons (PORT);
  

  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }


  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

  while (1)
    {
      int client;
      thData * td; 
      socklen_t length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);


      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	


	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	}
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);

		close (tdL.cl);
    free(arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
  int nr;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	if (read (tdL.cl, &nr,sizeof(int)) <= 0)
			{
			  printf("[Thread %d]\n",tdL.idThread);
			  perror ("Eroare la read() de la client.\n");
			
			}
	
  if((nr<=0) || (nr>10000)){
    printf("Something unexpected happened...");
    return;
  }
  printf ("[Thread %d]Marimea mesajului a fost receptionata...%d\n",tdL.idThread, nr);
  char comanda_primita[nr+1];
  if(read(tdL.cl, comanda_primita, sizeof(comanda_primita))<=0){
    printf("[Thread %d]\n",tdL.idThread);
    perror ("Eroare la read() de la client.\n");
  }
  printf ("[Thread %d]Comanda a fost receptionata...%s\n",tdL.idThread, comanda_primita);

  printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);
  MyProtocol.executa(comanda_primita, arg);
  printf("Comanda a fost executata! \n");
  MyProtocol.returneaza(arg, 0);


		    

}

// int comanda_valida(char* cmd){
//   string command_name = cmd;
//   for(int i=0;i<(int)command_name.size(); i++){
//     if(command_name[i]==' '){
//       command_name.erase(i);
//     }
//   }
//   string cmd_path = "/usr/bin/" + command_name;

//   printf("cmd_path = %s\n", cmd_path.c_str());

//   int fd = open(cmd_path.c_str(), O_RDONLY);

//   if(fd<-0){
//     return 0;
//   }
//   else{
//     close(fd);
//     return 1;
//   }
// }

void exec_cmd(const char* cmd, void* arg){
  struct thData tdL; 
	tdL= *((struct thData*)arg);
  Commandments command(cmd);
  string filename = "./Files/temp_" + to_string(getpid()) + "_" + to_string(pthread_self());
  command.creeate_path("temp_" + to_string(getpid()) + "_" + to_string(pthread_self()));

  int file_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(file_fd==-1){
    perror("EROARE LA DESCHIDEREA/CREEREA FILEI!");
  }

  if(command.GetTotalCMDs()==1){
    exec_sg_cmd(command, filename);
  }
  else{
    string filename_temp = filename+"__temp";
    exec_generic_cmd(command, 1, filename_temp);
    executa_mult_cmd(command, filename, filename_temp);
  }
    // int nr = strlen(cmd);

    // if((write(file_fd, &nr, sizeof(int)))==-1){
    //   perror("EROARE LA TRIMITEREA BITILOR!");
    // }
    // if((write(file_fd, cmd, strlen(cmd)))<=-1){
    //   perror("EROARE LA TRIMITEREA RASPUNSULUI!");
    // }
    // break;

  close(file_fd);
}


void return2cl(void* arg, int is_err){
	struct thData tdL; 
	tdL= *((struct thData*)arg);
  string filename = "./Files/temp_" + to_string(getpid()) + "_" + to_string(pthread_self());
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
      perror("SOMETHING UNEXPECTED HAPPENED!");
    }
  

  remove(filename.c_str());
}

void exec_sg_cmd(Commandments my_command, string filename){
  pid_t executare_comanda;
  string filename_temp = filename+"temp";
  int file_fd = open(filename_temp.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(file_fd==-1){
    perror("EROARE! FISIERUL TEMPORAR NU S-A PUTUT CREEA! GO BACK!");
    return;
  }

  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
    perror("EROARE! FISIER INEXISTENT! GO BACK!");
    return;
  }


  int status = 0;
  pid_t asteptare_fiu;

  if((executare_comanda=fork())==-1){
      perror("ERORARE LA CREEREA PROCESULUI!");
      return;
  }
  if(executare_comanda==0){
    dup2(file_fd, 1); 
    close(fd);
    close(file_fd);
    execv(my_command.return_path(1), my_command.char_convert(1));
    printf("Comanda '%s' nu exista!", my_command.return_cmd(1).c_str());
    exit(EXIT_FAILURE);
  }
  else
  {   


      asteptare_fiu = wait(&status);
      if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1");}

      struct stat stbuf;
      if(lstat(filename_temp.c_str(), &stbuf)==-1){
        perror("ERORARE LA PRELUAREA DATELOR!");
        exit(EXIT_FAILURE);
      }

      int file_size = stbuf.st_size;
      if(file_size>0){
        char* temp = new char[file_size+1];
        if(write(fd, &file_size, sizeof(file_size))<0){
          perror("EROARE LA SCRIEREA BITILOR IN FISIER!");
         exit(EXIT_FAILURE);
       }
       if(lseek(file_fd, 0, SEEK_SET)==-1){
         perror("EROARE LA SETAREA POZITIEI!");
         exit(EXIT_FAILURE);
       }
        if(read(file_fd, temp, file_size)<0){
          perror("EROARE LA CITIRE!!!");
          exit(EXIT_FAILURE);
        } 
       temp[file_size] = '\0';
       printf("TEXTUL: %s \n DE DIMENSIUNE %d \n", temp, file_size);

        if(write(fd, temp, file_size)<0){
          perror("EROARE LA SCRIEREA PE FISIERUL FINAL!");
          exit(EXIT_FAILURE);
        }
        delete[] temp;
      }
      else{
        char mesaj_generic[] = "Comanda a fost executata cu succes!";
        file_size = strlen(mesaj_generic)+1;
        if(write(fd, &file_size, sizeof(file_size))<0){
          perror("EROARE LA SCRIEREA BITILOR IN FISIER!");
         exit(EXIT_FAILURE);
       }
       if(write(fd, mesaj_generic, file_size)<0){
        perror("EROARE LA SCRIEREA PE FISIERUL FINAL!");
        exit(EXIT_FAILURE);
      }


      }

      close(fd);
      close(file_fd);
      remove(filename_temp.c_str());

  }


}


void exec_generic_cmd(Commandments my_command, int nr_cmd, string filename){


  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd==-1){
    perror("EROARE! FISIER INEXISTENT! GO BACK!");
    return;
  }

  int status = 0;
  pid_t asteptare_fiu;
  pid_t executare_comanda;
  string path = my_command.file_path();
  printf("fila: %s\n", path.c_str());

  if((executare_comanda=fork())==-1){
      perror("ERORARE LA CREEREA PROCESULUI!");
  }
  if(executare_comanda==0){
    dup2(fd, 1); 
    close(fd);
    execv(my_command.return_path(nr_cmd), my_command.char_convert(nr_cmd));
    remove(path.c_str());
    perror("ERORARE LA EXECUTARE!");
    exit(EXIT_FAILURE);
  }
  else
  {   
      asteptare_fiu = wait(&status);
      if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1"); exit(EXIT_FAILURE);}
      printf("Operatia %s a avut loc!\n", my_command.return_path(nr_cmd));
      close(fd);
      if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
        ofstream succes(path.c_str());
        succes.close();
      }
      else{
        remove(path.c_str());
     }
  }



}


void executa_mult_cmd(Commandments command, string filename, string filename_temp){
  int nr_cmd = 2;
  int operatie = command.return_operation(nr_cmd);
  string path = command.file_path();
  printf("Operatia este: %d\n", operatie);

  while (nr_cmd<=command.GetTotalCMDs())
  {
      switch (operatie)
      {
      case 0:
          pipeline(command, nr_cmd, filename_temp);
          break;

      case 1:
          AND_oftheworld(command, nr_cmd, filename_temp);
          break;

      case 2:
          ORwell(command, nr_cmd, filename_temp);
          break;

      case 3:
          DotCom(command, nr_cmd, filename_temp);
          break;
      
      default:
          printf("I don't care!\n");
          break;
      }

      nr_cmd++;
      operatie = command.return_operation(nr_cmd);
      printf("Operatia este: %d\nNr CMD este: %d\n", operatie, nr_cmd);
  }

  int file_fd = open(filename_temp.c_str(), O_RDWR);
  if(file_fd==-1){perror("EROARE LA DESCHIDEREA/CREEREA FISIERULUI!"); exit(EXIT_FAILURE);}
  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd==-1){perror("EROARE LA DESCHIDEREA/CREEREA FISIERULUI!"); exit(EXIT_FAILURE);}

  struct stat stbuf;
  if(lstat(filename_temp.c_str(), &stbuf)==-1){
    perror("ERORARE LA PRELUAREA DATELOR!");
    return;
  }

  int file_size = stbuf.st_size;
  if(file_size>0){
    char* temp = new char[file_size+1];
    if(write(fd, &file_size, sizeof(file_size))<0){
      perror("EROARE LA SCRIEREA BITILOR IN FISIER!");
     exit(EXIT_FAILURE);
   }
   if(lseek(file_fd, 0, SEEK_SET)==-1){
     perror("EROARE LA SETAREA POZITIEI!");
     exit(EXIT_FAILURE);
   }
    if(read(file_fd, temp, file_size)<0){
      perror("EROARE LA CITIRE!!!");
      exit(EXIT_FAILURE);
    } 
   temp[file_size] = '\0';
   //printf("TEXTUL: %s \n DE DIMENSIUNE %d \n", temp, file_size);

    if(write(fd, temp, file_size)<0){
      perror("EROARE LA SCRIEREA PE FISIERUL FINAL!");
      exit(EXIT_FAILURE);
    }
    delete[] temp;
  }
  else{
    char mesaj_generic[] = "...";
    file_size = strlen(mesaj_generic)+1;
    if(write(fd, &file_size, sizeof(file_size))<0){
      perror("EROARE LA SCRIEREA BITILOR IN FISIER!");
     exit(EXIT_FAILURE);
   }
   if(write(fd, mesaj_generic, file_size)<0){
    perror("EROARE LA SCRIEREA PE FISIERUL FINAL!");
    exit(EXIT_FAILURE);
  }


  }

  //printf("Comanda a fost executata cu succes!\n");
  remove(filename_temp.c_str());
  remove(path.c_str());


}

void pipeline(Commandments cmd, int nr_cmd, string filename){
      //printf("COMANDA1 ESTE: %s \n COMANDA2 ESTE: %s \n", cmd1, cmd2);

      pid_t proces;
      pid_t  asteptare_fiu;
      string filenametemp = filename+"__temporary";
      int status;
      string path = cmd.file_path();


      int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      if(fd==-1){
          perror("EROARE LA DESCHIDEREA FISIERULUI!!");
          exit(EXIT_FAILURE);
      }
      int file_fd = open(filenametemp.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(file_fd==-1){
          perror("EROARE LA DESCHIDEREA FISIERULUI!!");
          exit(EXIT_FAILURE);
      }


      lseek(fd, 0, SEEK_SET);
      if((proces=fork())==-1){
          perror("EROARE LA FURCULITA!!");
          exit(EXIT_FAILURE);
      }
      if(proces==0){
          dup2(fd, 0);
          dup2(file_fd, 1);
          close(fd);
          close(file_fd);
          execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
          perror("ERORARE! COMANDA NU A PUTUT FI EXECUTATA!!");
          exit(EXIT_FAILURE);
      }
      else{
          asteptare_fiu = wait(&status);
          if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1"); exit(EXIT_FAILURE);}

          sync();

          close(fd);
          close(file_fd);

          if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
            ofstream succes(path.c_str());
            succes.close();
          }
          else{
            remove(path.c_str());
         }
          remove(filename.c_str());
          if(rename(filenametemp.c_str(), filename.c_str()) == -1){
            perror("EROARE LA RENAME!");
            exit(EXIT_FAILURE);
        }


      }
}

void AND_oftheworld(Commandments cmd, int nr_cmd, string filename){

  ///idee: implementarea unei variabile "previous" care tine cont daca codul anterior a esuat sau nu. Ar fi o idee sa includ previous direct in Commandments.

  string path = cmd.file_path();
  int file_exists = open(path.c_str(), O_RDONLY);
  printf("%s\n", path.c_str());
  if(file_exists==-1){
    printf("COMANDA ANTERIOARA A ESUAT!");
    return;
  }
  close(file_exists);

  pid_t proces;
  pid_t  asteptare_fiu;
  int status;

  printf("fila: %s\n", path.c_str());


  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
  }


  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
      dup2(fd, 0);
      dup2(fd, 1);
      close(fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      perror("ERORARE! COMANDA NU A PUTUT FI EXECUTATA!!");
      exit(EXIT_FAILURE);
  }
  else{
      asteptare_fiu = wait(&status);
      if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1"); exit(EXIT_FAILURE);}
      sync();
      close(fd);

      if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
        ofstream succes(path.c_str());
        succes.close();
      }
      else{
        remove(path.c_str());
     }



  }


}

void ORwell(Commandments cmd, int nr_cmd, string filename){



  string path = cmd.file_path();
  int file_exists = open(path.c_str(), O_RDONLY);
  printf("%s\n", path.c_str());
  if(file_exists!=-1){
    printf("COMANDA ANTERIOARA NU A ESUAT!");
    return;
  }
  close(file_exists);

  pid_t proces;
  pid_t  asteptare_fiu;
  int status;

  printf("fila: %s\n", path.c_str());


  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
  }


  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
      dup2(fd, 0);
      dup2(fd, 1);
      close(fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      perror("ERORARE! COMANDA NU A PUTUT FI EXECUTATA!!");
      exit(EXIT_FAILURE);
  }
  else{
      asteptare_fiu = wait(&status);
      if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1"); exit(EXIT_FAILURE);}
      sync();
      close(fd);

      if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
        ofstream succes(path.c_str());
        succes.close();
      }
      else{
        remove(path.c_str());
     }



  }


}

void DotCom(Commandments cmd, int nr_cmd, string filename){



  string path = cmd.file_path();
  pid_t proces;
  pid_t  asteptare_fiu;
  int status;


  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
  }


  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
      dup2(fd, 0);
      dup2(fd, 1);
      close(fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      perror("ERORARE! COMANDA NU A PUTUT FI EXECUTATA!!");
      exit(EXIT_FAILURE);
  }
  else{
      asteptare_fiu = wait(&status);
      if(asteptare_fiu==-1){perror("Eroare la asteptarea fiului1"); exit(EXIT_FAILURE);}
      sync();
      close(fd);

      if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
        ofstream succes(path.c_str());
        succes.close();
      }
      else{
        remove(path.c_str());
     }



  }


}