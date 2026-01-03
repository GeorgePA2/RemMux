#include "Commandments.h"

typedef struct info_operatii
{
  string nume;
  int operatia;
  int pozitie_init;
  int pozitie_fin;
}info_operatii;

typedef struct pozitii
{
  int gasit;
  int pozitie_initiala;
  int pozitie_finala;
}pozitii;

string separa_comenzi(const char* cmd, int pozitie_start, int pozitie_finala);
pozitii find_sep(const char* string_seq, const char* sep, int pozitie_start);
info_operatii op_type(const char* cmd, int poz_start);
void pregateste_comanda(const char* cmd, char** &vector_de_comenzi);
vector<string> separate_inside(string cmd);

Commandments::Commandments(const char *cmd)
{
    this->succes_path = "./Success/";
    this->total_commands = 0;
    string cmd_str;
    info_operatii op_info = op_type(cmd, 0);
    int operatie = op_info.operatia;
    printf("Prima operatie este: %d\n", operatie);
    cmd_str = separa_comenzi(cmd, op_info.pozitie_init, op_info.pozitie_fin);
    //printf("CMD1 este: %s \n", cmd_str.c_str());
    this->Commands.push_back(cmd_str);
    this->Comenzi_separate.push_back(separate_inside(cmd_str));
    this->total_commands++;
    //printf("pozitia de start %d \n elementul de pe pozitiea de start %c \n comanda este %s \n Operatia corespunzatoare : %d \n", op_info.pozitie_init, cmd[op_info.pozitie_init], cmd_str.c_str(), op_info.operatia);
    while (operatie!=-1)
    {

        op_info = op_type(cmd, op_info.pozitie_fin + op_info.nume.size());
        cmd_str = separa_comenzi(cmd, op_info.pozitie_init+1, op_info.pozitie_fin);
        this->Commands.push_back(cmd_str);
        this->Comenzi_separate.push_back(separate_inside(cmd_str));
        this->total_commands++;
        this->ordinea_operatiilor.push_back(operatie);
        //printf("pozitia de start %d \n elementul de pe pozitiea de start %c \n comanda este %s \n Operatia corespunzatoare : %d \n", op_info.pozitie_init, cmd[op_info.pozitie_init], cmd_str.c_str(), op_info.operatia);
        operatie = op_info.operatia;
    }
    //   printf("Operatiile sunt: ");
    // for(int j=0;j<(int)ordinea_operatiilor.size();j++){
    //     printf("%d ", ordinea_operatiilor[j]);
    // }
    // printf("\n");
}

Commandments::~Commandments()
{
    this->Comenzi_separate.clear();
    this->Commands.clear();
    this->total_commands = 0;
    this->ordinea_operatiilor.clear();
}

int Commandments::GetTotalCMDs()
{
    return this->total_commands;
}

char **Commandments::char_convert(int nr_cmd)
{
    vector<string> argumente = this->Comenzi_separate[nr_cmd-1];
    char** vector_exec = new char*[argumente.size()+1];
    int i = 0;
    for(const auto &x : argumente){
        vector_exec[i] = new char[argumente[i].size()+1];
        strcpy(vector_exec[i], x.c_str());
        //printf("argumentul %d este %s\n", i, x.c_str());
        i++;
    }
    vector_exec[i] = nullptr;
    argumente.clear();

    return vector_exec;
}

char *Commandments::return_path(int nr_cmd)
{

    string path_real = "/usr/bin/" + this->Comenzi_separate[nr_cmd-1][0];
    char* path = new char[path_real.size()];
    strcpy(path, path_real.c_str());
    return path;
}

int Commandments::return_operation(int nr_cmd)
{
    if((nr_cmd<=1) && nr_cmd>this->total_commands){
        return -1;
    }

    return this->ordinea_operatiilor[nr_cmd-2];
}

string Commandments::return_cmd(int nr_cmd)
{
    return this->Commands[nr_cmd-1];
}

bool Commandments::is_next_file(int nr_cmd)
{
    if(nr_cmd>0 && nr_cmd<GetTotalCMDs()){
      if(return_operation(nr_cmd+1>=4) && (return_operation(nr_cmd)>=1 && return_operation(nr_cmd)<4)){
        return true;
      }
      if(return_operation(nr_cmd+1)>=5 && return_operation(nr_cmd==0)){
        return true;
      }
    }
    return false;
}

string Commandments::file_path()
{
    return this->succes_path;
}

void Commandments::creeate_path(string file)
{
  this->succes_path += file;
}


string separa_comenzi(const char* cmd, int pozitie_start, int pozitie_finala){
    string cmd_str;
    for(int i = pozitie_start; i<=pozitie_finala;i++){
        cmd_str += cmd[i];
    }
    return cmd_str;
  }

pozitii find_sep(string string_seq, string sep, int pozitie_start){
  
    long unsigned int i = string_seq.find(sep, pozitie_start);

    if(i!=string::npos){
      pozitii mypositions{
        .gasit=1,
        .pozitie_initiala = pozitie_start,
        .pozitie_finala = (int)i-1
      };
      return mypositions;
    }
  
    pozitii mypositions{
      .gasit = 0,
      .pozitie_initiala = pozitie_start,
      .pozitie_finala = (int)(string_seq.size()-1)
    };
  
  
    return mypositions;
  }

  info_operatii op_type(const char* cmd, int poz_start){
    vector<string> separatori = {" | ", " && ", " || ", " ; ", " > ", " < ", " 2> "};
    string commander = cmd;
    pozitii pos;
    int poz_min = strlen(cmd)+10;
    int i = 0;

    info_operatii result{
      .nume = "N/A",
      .operatia = -1,
      .pozitie_init = poz_start,
      .pozitie_fin = (int)strlen(cmd)
    };
    for(const auto& x : separatori){
      pos=find_sep(commander, x, poz_start);
      if((pos.gasit) && (pos.pozitie_finala<poz_min)){
        result.nume = x;
        result.operatia = i;
        result.pozitie_init = pos.pozitie_initiala;
        result.pozitie_fin = pos.pozitie_finala;
        poz_min = pos.pozitie_finala;
        //printf("Operatia este: %s, i este %d \n", x.c_str(), i);
      }
      i++;
      
    }

    return result;
  }

  vector<string> separate_inside(string cmd){
    //printf("Comanda este: %s\n", cmd.c_str());
    vector<string> argumente;
    char* temp = new char[cmd.size()+1];
    strcpy(temp, cmd.c_str());
    char * p = strtok(temp , " ");
    //printf("Argumenteul este format din: ");
    while(p!=NULL){
      //printf("%s ", p);
      argumente.push_back(p);
      p = strtok(NULL, " ");
    }
    //printf("\n");
    delete[] temp;
    return argumente;

  }