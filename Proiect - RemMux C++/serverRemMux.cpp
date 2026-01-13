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
#include <signal.h>



#define PORT 2908
using namespace std;

extern int errno;

typedef struct thData{
	int idThread; 
	int cl; 

}thData;

string abs_filename;


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

void executa_mult_cmd(Commandments command, string filename, string filename_temp, int start);
void exec_generic_cmd(Commandments my_command, int nr_cmd, string filename, string input, int special_cmd);
void handle_execution(Commandments cmd);
void exec_sg_cmd(Commandments my_command, string filename);


bool ORwell(Commandments cmd, int nr_cmd, string filename, int special_file, string output);
void pipeline(Commandments cmd, int nr_cmd, string filename, string input, int special_file);
bool AND_oftheworld(Commandments cmd, int nr_cmd, string filename, int special_file, string output);
bool DotCom(Commandments cmd, int nr_cmd, string filename, int special_file, string output);
void Redirect_SingleOutput(string OG_file, string output);

bool valid_output(string file);

bool custom_cd(vector<string> arguments, string filename, int special_file, string output, Commandments &cmd);
bool proceseaza_custom_cmd(Commandments cmd, int nr_cmd ,string filename, int special_file, string output);

bool timeout(pid_t proces, int &status, int timeout_timer){
  int seconds_passed = 0;
  while(true){
    pid_t result = waitpid(proces, &status, WNOHANG);
    if(result==proces){
      printf("[DEBUG]Gata!\n");
      return true;
      break;
    }
    else if (result==-1)
    {
      perror("ERORARE LA ASTEPTAREA PROCESULUI!");
      printf("[DEBUG]Gata!\n");
      return false;
      break; 
    }
    if(seconds_passed>=timeout_timer*1000){
      kill(-proces, SIGKILL);
      waitpid(proces, &status, 0);
      return false;
    }
    if(result==0){
     // printf("[DEBUG]Goodnight sleep!\n");
      usleep(100000);
  }
    seconds_passed += 100;
    //printf("[DEBUG]Au trecut%d secunde!\n", seconds_passed);
  }
}


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
  mkdir("./Files", 0777);
  


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
  

  char current_dir[4096];
  if (!getcwd(current_dir, sizeof(current_dir))) {
    perror("EROARE getcwd!");
    return;
  }
  abs_filename = string(current_dir);
  string filename = abs_filename +  "/Files/temp_" + to_string(getpid()) + "_" + to_string(pthread_self());
  command.creeate_path("temp_" + to_string(getpid()) + "_" + to_string(pthread_self()), abs_filename);

  int file_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
  if(file_fd==-1){
    perror("EROARE LA DESCHIDEREA/CREEREA FILEI!");
  }

  if(command.GetTotalCMDs()==1){
    exec_sg_cmd(command, filename);
  }
  else{
    string filename_temp = filename+"__temp";
    int starting_point = 2;
    if(command.return_operation(2)==5){
      exec_generic_cmd(command, 1, filename_temp, command.return_cmd(2), 1);
      starting_point = 3;
    }
    else if (command.return_operation(2)==6)
    {
      exec_generic_cmd(command, 1, filename_temp, command.return_cmd(2), 2);
      starting_point = 3;
    }
    else{
      exec_generic_cmd(command, 1, filename_temp, filename_temp, 0);
      starting_point = 2;
    }
    executa_mult_cmd(command, filename, filename_temp, starting_point);

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

  string filename = abs_filename + "/Files/temp_" + to_string(getpid()) + "_" + to_string(pthread_self());
  int sizeofmsg;

  printf("[DEBUG] Filename is: %s\n", filename.c_str());

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

  if(my_command.detect_custom_cmd(1)=="N/A"){
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


  printf("Nu am avut comanda custom!\n");
  int status = 0;
  if((executare_comanda=fork())==-1){
      perror("ERORARE LA CREEREA PROCESULUI!");
      return;
  }
  if(executare_comanda==0){
    setpgid(0, 0); 
    dup2(file_fd, 1); 
    close(fd);
    close(file_fd);
    execv(my_command.return_path(1), my_command.char_convert(1));
    printf("Comanda '%s' nu exista!\n", my_command.return_cmd(1).c_str());
    exit(EXIT_FAILURE);
  }
  else
  {   



      bool fiul_asteptator = timeout(executare_comanda, status, 5);
      if(fiul_asteptator==false){

        string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
        if((write(file_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
          perror("ERORARE LA TIMEOUT!");
          exit(EXIT_FAILURE);
        }
      }

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
        char mesaj_generic[] = "Comanda a fost executata cu succes!\n";
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

  else{
    proceseaza_custom_cmd(my_command, 1, filename_temp, 0, filename_temp);
    printf("Am avut comanda custom!\n");
    int file_fd = open(filename_temp.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if(file_fd==-1){
      printf("[DEBUG]File name: %s are probleme\n", filename_temp.c_str());
      perror("EROARE! FISIERUL TEMPORAR NU S-A PUTUT CREEA! GO BACK!");
      exit(EXIT_FAILURE);
    }

    printf("[DEBUG]File name: %s\n", filename_temp.c_str());
  
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if(fd==-1){
      printf("[DEBUG]File name: %s are probleme\n", filename.c_str());
      perror("EROARE! FISIER INEXISTENT! GO BACK!");
      exit(EXIT_FAILURE);
    }

    printf("[DEBUG]File name: %s\n", filename.c_str());

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
      char mesaj_generic[] = "Comanda a fost executata cu succes!\n";
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


void exec_generic_cmd(Commandments my_command, int nr_cmd, string filename, string input, int special_cmd){


  if(my_command.detect_custom_cmd(nr_cmd)=="N/A"){
  int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd==-1){
    perror("EROARE! FISIER INEXISTENT! GO BACK!");
    return;
  }
  int fd_input;
  int fd_err;
  if(special_cmd==1){
    fd_input = open(input.c_str(), O_RDONLY);
    if(fd_input==-1){
      string mesaj_eroare = "Permission denied!";
      if((write(fd, mesaj_eroare.c_str(), mesaj_eroare.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIER!!");
        exit(EXIT_FAILURE);
      }
      close(fd);
      return;
    }
  }
  if(special_cmd==2){
    fd_err = open(input.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_input==-1){
      string mesaj_eroare = "Permission denied!\n";
      if((write(fd, mesaj_eroare.c_str(), mesaj_eroare.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIER!!");
        exit(EXIT_FAILURE);
      }
      close(fd);
      return;
    }
  }

  int status = 0;
  pid_t executare_comanda;
  string path = my_command.file_path();
  printf("fila: %s\n", path.c_str());

  if((executare_comanda=fork())==-1){
      perror("ERORARE LA CREEREA PROCESULUI!");
  }
  if(executare_comanda==0){
    setpgid(0, 0);
    dup2(fd, 1); 
    close(fd);
    if(special_cmd==1){
      dup2(fd_input, 0);
      close(fd_input);
    }
    if(special_cmd==2){
      dup2(fd_err, 2);
      close(fd_err);
    }
    execv(my_command.return_path(nr_cmd), my_command.char_convert(nr_cmd));
    remove(path.c_str());
    if(special_cmd==2){
    fprintf(stderr, "Comanda %s nu exista\n", my_command.return_cmd(nr_cmd).c_str());
    }
    else{
      printf("Comanda %s nu exista\n", my_command.return_cmd(nr_cmd).c_str());
    }
    exit(EXIT_FAILURE);
  }
  else
  {   
    bool fiul_asteptator = timeout(executare_comanda, status, 5);
    if(fiul_asteptator==false){

      string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
      int new_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if((write(new_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
        perror("ERORARE LA TIMEOUT!");
        exit(EXIT_FAILURE);
      }
      close(new_fd);
      return;
    }
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

else{
  proceseaza_custom_cmd(my_command, nr_cmd, filename, special_cmd, filename);
  remove(my_command.file_path().c_str());
}



}


void executa_mult_cmd(Commandments command, string filename, string filename_temp, int start){
  int nr_cmd = start;
  int operatie = command.return_operation(nr_cmd);
  string path = command.file_path();
  string output;
  string lastfile = filename_temp;
  int special_operation = 0;
  printf("[DEBUG]Operatia este: %d\n", operatie);

  while (nr_cmd<=command.GetTotalCMDs())
  {
    if(command.is_next_file(nr_cmd)){
      output = command.return_cmd(nr_cmd+1);
      special_operation = command.return_operation(nr_cmd+1)-3;
    }
    else{
      output = filename_temp;
      special_operation = 0;
    }

      switch (operatie)
      {
      case 0:
          printf("[DEBUG]Output is : %s\n[DEBUG]Special operation = %d\n", output.c_str(), special_operation);
          pipeline(command, nr_cmd, filename_temp, output, special_operation);
          lastfile = filename_temp;
          break;

      case 1:
          printf("[DEBUG]Output is : %s\n", output.c_str());
          if(AND_oftheworld(command, nr_cmd, filename_temp, special_operation, output)==true){
          lastfile = output;
          }
          break;

      case 2:
          printf("[DEBUG]Output is : %s\n", output.c_str());
          if(ORwell(command, nr_cmd, filename_temp, special_operation, output)==true){
          lastfile = output;
          }
          break;

      case 3:
          printf("[DEBUG]Output is : %s\n", output.c_str());
          if(DotCom(command, nr_cmd, filename_temp, special_operation, output)==true){
          lastfile = output;
          }
          break;

      case 4:{
          string output = command.return_cmd(nr_cmd);
          printf("[DEBUG]Output is : %s\n", output.c_str());
          printf("[DEBUG]Lastfile is : %s\n", lastfile.c_str());
          Redirect_SingleOutput(lastfile, output);
          lastfile = output;
          break;
      }
      case 5:
          break;
      
      default:
          printf("I don't care!\n");
          break;
      }

      if(special_operation>0){
        nr_cmd += 2;
      }
      else{
        nr_cmd++;
      }
      operatie = command.return_operation(nr_cmd);
      printf("[DEBUG]Operatia este: %d\n[DEBUG]Nr CMD este: %d\n", operatie, nr_cmd);
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
    char mesaj_generic[] = "Comanda a fost executata cu succes!\n";
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

void pipeline(Commandments cmd, int nr_cmd, string filename, string input, int special_file){

  //printf("COMANDA1 ESTE: %s \n COMANDA2 ESTE: %s \n", cmd1, cmd2);
  pid_t proces;
  string filenametemp = filename+"__temporary";
  int status;
  string path = cmd.file_path();


  int fd;
  int fd_err;

  if(special_file==2){
    printf("[DEBUG] Avem input in pipeline, anume %s\n", input.c_str());
    fd = open(input.c_str(), O_RDONLY);
    if(fd==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return;
    }
  }

  else if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", input.c_str());
    fd_err = open(input.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return;
    }

    fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);

     if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }

  }
  

  else{
    fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      if(fd==-1){
       perror("EROARE LA DESCHIDEREA FISIERULUI!!");
       exit(EXIT_FAILURE);
    }
  }

  int file_fd = open(filenametemp.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

  if(file_fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
  }
  lseek(fd, 0, SEEK_SET);

  if(cmd.detect_custom_cmd(nr_cmd)=="N/A"){


  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
      setpgid(0, 0);
      dup2(fd, 0);
      dup2(file_fd, 1);
      if(special_file==3){
        dup2(fd_err, 2);
        close(fd_err);
      }
      close(fd);
      close(file_fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      if(special_file==3){
      fprintf(stderr, "Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
      }
      else{
        printf("Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
      }
      exit(EXIT_FAILURE);
  }
  else{
    bool fiul_asteptator = timeout(proces, status, 5);
    if(fiul_asteptator==false){

      string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
      int new_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if((write(new_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
        perror("ERORARE LA TIMEOUT!");
        exit(EXIT_FAILURE);
      }
      close(new_fd);
      remove(path.c_str());
      remove(filenametemp.c_str());
      return;
    }
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

else{
  proceseaza_custom_cmd(cmd, nr_cmd, filenametemp, special_file, input);
  remove(path.c_str());
  remove(filename.c_str());
  if(rename(filenametemp.c_str(), filename.c_str()) == -1){
    perror("EROARE LA RENAME!");
    exit(EXIT_FAILURE);

}

}

}

bool AND_oftheworld(Commandments cmd, int nr_cmd, string filename, int special_file, string output){

  string path = cmd.file_path();
  int file_exists = open(path.c_str(), O_RDONLY);
  printf("%s\n", path.c_str());
  if(file_exists==-1){
    printf("[DEBUG]COMANDA ANTERIOARA A ESUAT!\n");
    return false;
  }
  close(file_exists);
  int fd;
  int fd_input;
  int fd_err;
  if(cmd.detect_custom_cmd(nr_cmd)=="N/A"){
  printf("[DEBUG] special file = %d\n", special_file);

  if(special_file==1){
    fd = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  else{
    fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666); 
    if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }
  }

  if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", output.c_str());
    fd_err = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }

  if(special_file==2){
    fd_input = open(output.c_str(), O_RDONLY); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd_input==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
    printf("[DEBUG] FIla %s a putut fi deschisa!\n", output.c_str());
  }



  pid_t proces;
  int status;

  printf("fila: %s\n", path.c_str());

  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
      setpgid(0, 0);
      if(special_file==2){
        dup2(fd_input, 0);
      }
      else{
      dup2(fd, 0);
      }
      if(special_file==3){
        dup2(fd_err, 2);
        close(fd_err);
      }
      dup2(fd, 1);
      close(fd);
      if(special_file==2){
        close(fd_input);
      }
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      if(special_file==3){
        fprintf(stderr, "Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
        else{
          printf("Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
      exit(EXIT_FAILURE);
  }
  else{
    bool fiul_asteptator = timeout(proces, status, 5);
    if(fiul_asteptator==false){

      string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
      int new_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      if((write(new_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
        perror("ERORARE LA TIMEOUT!");
        exit(EXIT_FAILURE);
      }
      close(new_fd);
      remove(path.c_str());
      return false;
    }
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

  return true;
}

else{
  proceseaza_custom_cmd(cmd, nr_cmd, filename, special_file, output);
  remove(path.c_str());
  return false;
}


}

bool ORwell(Commandments cmd, int nr_cmd, string filename, int special_file, string output){



  string path = cmd.file_path();
  int file_exists = open(path.c_str(), O_RDONLY);
  printf("%s\n", path.c_str());
  if(file_exists!=-1){
    printf("COMANDA ANTERIOARA NU A ESUAT!");
    return false;
  }
  close(file_exists);

  if(cmd.detect_custom_cmd(nr_cmd)=="N/A"){

  pid_t proces;
  int status;

  printf("fila: %s\n", path.c_str());


  int fd;
  int fd_input;
  int fd_err;

  if(special_file==1){
    fd = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  else{
    fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666); 
    if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }
  }

  if(special_file==2){
    fd_input = open(output.c_str(), O_RDONLY); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd_input==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
    printf("[DEBUG] FIla %s a putut fi deschisa!\n", output.c_str());
  }

  if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", output.c_str());
    fd_err = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }


  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
    setpgid(0, 0);
    if(special_file==2){
      dup2(fd_input, 0);
      close(fd_input);
    }
    else{
      dup2(fd, 0);
    }
    if(special_file==3){
      dup2(fd_err, 2);
    }
      dup2(fd, 1);
      close(fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      if(special_file==3){
        fprintf(stderr, "Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
        else{
          printf("Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
      exit(EXIT_FAILURE);
  }
  else{
    bool fiul_asteptator = timeout(proces, status, 5);
    if(fiul_asteptator==false){

      string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
      int new_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      if((write(new_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
        perror("ERORARE LA TIMEOUT!");
        exit(EXIT_FAILURE);
      }
      close(new_fd);
      remove(path.c_str());
      return false;
    }
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

  return true;
  }
  else{
    proceseaza_custom_cmd(cmd, nr_cmd, filename, special_file, output);
    remove(path.c_str());
    return false;
  }

}

bool DotCom(Commandments cmd, int nr_cmd, string filename, int special_file, string output){



  string path = cmd.file_path();
  pid_t proces;
  int status;

  if(cmd.detect_custom_cmd(nr_cmd)=="N/A"){

  int fd;
  int fd_input;
  int fd_err;

  if(special_file==1){
    fd = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  else{
    fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666); 
    if(fd==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }
  }

  if(special_file==2){
    fd_input = open(output.c_str(), O_RDONLY); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd_input==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
    printf("[DEBUG] FIla %s a putut fi deschisa!\n", output.c_str());
  }

  if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", output.c_str());
    fd_err = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }


  lseek(fd, 0, SEEK_SET);
  if((proces=fork())==-1){
      perror("EROARE LA FURCULITA!!");
      exit(EXIT_FAILURE);
  }
  if(proces==0){
    setpgid(0, 0);
    if(special_file==2){
      dup2(fd_input, 0);
      close(fd_input);
    }
    else{
      dup2(fd, 0);
    }
      dup2(fd, 1);
    if(special_file==3){
      dup2(fd_err, 2);
      close(fd_err);
    }
      close(fd);
      execv(cmd.return_path(nr_cmd), cmd.char_convert(nr_cmd));
      remove(path.c_str());
      if(special_file==3){
        fprintf(stderr, "Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
        else{
          printf("Comanda %s nu exista!\n", cmd.return_cmd(nr_cmd).c_str());
        }
      exit(EXIT_FAILURE);
  }
  else{
    bool fiul_asteptator = timeout(proces, status, 5);
    if(fiul_asteptator==false){

      string timeout_msg = "Comanda nu s-a putut executa intr-un timp suficient\n";
      int new_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      if((write(new_fd, timeout_msg.c_str(), timeout_msg.size()))==-1){
        perror("ERORARE LA TIMEOUT!");
        exit(EXIT_FAILURE);
      }
      close(new_fd);
      remove(path.c_str());
      return false;
    }
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

  return true;
}
else{
  proceseaza_custom_cmd(cmd, nr_cmd, filename, special_file, output);
  remove(path.c_str());
  return false;
}


}

void Redirect_SingleOutput(string OG_file, string output){
    int fd_OG = open(OG_file.c_str(), O_RDWR | O_APPEND, 0666);
    if(fd_OG==-1){
      perror("EROARE LA DESCHIDEREA FILEI!");
      exit(EXIT_FAILURE);
    }
    int fd_output = open(output.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if(fd_output==-1){
      string no_permission = "Permission denied!\n";
      close(fd_OG);
      int fd_trunc = open(OG_file.c_str(), O_WRONLY | O_TRUNC);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA FILEI DE TRUNCHIERE!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size())==-1)){
        perror("EROARE LA SCRIEREA IN FISIERUL Original!");
        exit(EXIT_FAILURE);
      }
      close(fd_output);
      close(fd_trunc);
      return;
    }

    struct stat stbuf;
    if(lstat(OG_file.c_str(), &stbuf)==-1){
      perror("ERORARE LA PRELUAREA DATELOR!");
      return;
    }
  
    int file_size = stbuf.st_size;

    if(file_size>0){
      char* temp = new char[file_size+1];
     if(lseek(fd_OG, 0, SEEK_SET)==-1){
       perror("EROARE LA SETAREA POZITIEI!");
       exit(EXIT_FAILURE);
     }
      if(read(fd_OG, temp, file_size)<0){
        perror("EROARE LA CITIRE!!!");
        exit(EXIT_FAILURE);
      } 
     temp[file_size] = '\0';
      if(write(fd_output, temp, file_size)<0){
        perror("EROARE LA SCRIEREA PE FISIERUL FINAL!");
        exit(EXIT_FAILURE);
      }
      delete[] temp;
      close(fd_output);
      close(fd_OG);

      int fd_trunc = open(OG_file.c_str(), O_WRONLY | O_TRUNC);
      close(fd_trunc);
    }
    
}

bool valid_output(string file)
{
  int fd = open(file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd==-1){
    return false;
  }
  else{
    return true;
  }
}

bool proceseaza_custom_cmd(Commandments cmd, int nr_cmd ,string filename, int special_file, string output){
  int fd_output, fd_err;
  if(special_file==1){
    fd_output = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd_output==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  else{
    fd_output = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666); 
    if(fd_output==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }
  }
  if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", output.c_str());
    fd_err = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  if(cmd.detect_custom_cmd(nr_cmd)!="N/A"){
    string refusal = "Nu aveti permisiunea de a folosi comanda " + cmd.detect_custom_cmd(nr_cmd) + "!\n";
    if(special_file==3){
      if((write(fd_err, refusal.c_str(), refusal.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI!");
        exit(EXIT_FAILURE);
      }
    }
    else{
      if((write(fd_output, refusal.c_str(), refusal.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI!");
        exit(EXIT_FAILURE);
      }
    }

  }
  return false;
}

bool custom_cd(vector<string> arguments, string filename, int special_file, string output, Commandments &cmd){
  string path;
  string target = getenv("HOME");
  int fd_err;
  int fd_output;

  if(special_file==1){
    fd_output = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666); //ok deci mesajul de eroare trebuie gandit astfel incat sa stiu ce sa fac in caz ca fila nu exista...
    if(fd_output==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
      string no_permission = "Permission denied!";
      if(fd_trunc==-1){
        perror("EROARE! NU SE POATE DESCHIDE FILA ORIGINALA!!");
        exit(EXIT_FAILURE);
      }
      if((write(fd_trunc, no_permission.c_str(), no_permission.size()))==-1){
        perror("EROARE LA SCRIEREA PE FISIERUL ORIGINAL!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }
  else{
    fd_output = open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666); 
    if(fd_output==-1){
      perror("EROARE LA DESCHIDEREA FISIERULUI!!");
      exit(EXIT_FAILURE);
    }
  }
  if (special_file==3)
  {
    printf("[DEBUG] Avem input in pipeline, anume %s\n", output.c_str());
    fd_err = open(output.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fd_err==-1){
      int fd_trunc = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
      if(fd_trunc==-1){
        perror("EROARE LA CREEREA TRUNCHIERII IN PIPELINE!!");
        exit(EXIT_FAILURE);
      }
      string msg_err = "Permision denied!\n";
      if((write(fd_trunc, msg_err.c_str(), msg_err.size()))==-1){
        perror("EROARE LA TRANSMITEREA MESAJULUI DE EROARE!");
        exit(EXIT_FAILURE);
      }
      close(fd_trunc);
      return true;
    }
  }

  char current_directory[4096];
  if (!getcwd(current_directory, sizeof(current_directory))) {
    string err = string("getcwd error: ") + strerror(errno) + "\n";
    if(special_file==3){
      if((write(fd_err, err.c_str(), err.size()))==-1){
        perror("EROARE LA WRITE ERR IN CD!");
        exit(EXIT_FAILURE);
        }
        close(fd_err);
      }
    else{
      if((write(fd_output, err.c_str(), err.size()))==-1){
        perror("EROARE LA WRITE ERR IN CD!");
        exit(EXIT_FAILURE);
      }
      close(fd_output);
    }
    return false;
  }

  string prev_dir = current_directory;



  if(arguments.size()>1){
  path = arguments[1];



  if(path=="~"){
    target = getenv("HOME");
  }
  else if (path[0] == '~')
  {
    target = string(getenv("HOME")) + path.substr(1);
  }
  else if (path == "-")
  {
    target = cmd.return_prv_directory();
  }
  // else if (path==".." && cmd.return_prv_directory()=="~")
  // {
  //   cmd.change_prv_directory(prev_dir);
  //   output = "No permission to go back\n";
  //   if(special_file==1){
  //     if((write(fd_err, output.c_str(), output.size()))==-1){
  //       perror("EROARE LA WRITE ERR IN CD!");
  //       exit(EXIT_FAILURE);
  //       }
  //     }

  //   return false;
  // }
  
  else{
    target = path;

  }
    cmd.change_prv_directory(prev_dir);

  }

    if(chdir(target.c_str()) != 0){
      string err = string("cd: ") + strerror(errno) + "\n";
      if(special_file==3){
        if((write(fd_err, err.c_str(), err.size()))==-1){
          perror("EROARE LA WRITE ERR IN CD!");
          exit(EXIT_FAILURE);
          }
          close(fd_err);
        }
      else{
        if((write(fd_output, err.c_str(), err.size()))==-1){
          perror("EROARE LA WRITE ERR IN CD!");
          exit(EXIT_FAILURE);
        }
        close(fd_output);
      }
      printf("[DEBUG] cd NU a avut loc!\n");
      return false;
    }
    else{

    string generic_msg = "Comanda cd a avut loc!\n";
      if((write(fd_output, generic_msg.c_str(), generic_msg.size()))==-1){
        perror("EROARE LA WRITE ERR IN CD!");
        exit(EXIT_FAILURE);
      }
    }
    close(fd_output);

    printf("[DEBUG] cd a avut loc!\n");
    return true;


}