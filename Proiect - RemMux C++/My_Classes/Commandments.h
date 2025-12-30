
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

using namespace std;

class Commandments
{
private:
    vector<string> Commands;
    vector<vector<string>> Comenzi_separate; 
    vector<int> ordinea_operatiilor;
    int total_commands = 0;
    string succes_path;
public:
    Commandments(const char* cmd);
    ~Commandments();
    int GetTotalCMDs();
    char** char_convert(int nr_cmd);
    char* return_path(int nr_cmd);
    int return_operation(int nr_cmd);
    string return_cmd(int nr_cmd);
    bool is_next_file(int nr_cmd);
    string file_path();
    void creeate_path(string file);
};
