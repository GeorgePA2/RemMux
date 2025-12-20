
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

using namespace std;

class Commandments
{
private:
    vector<string> Commands;
    vector<vector<string>> Comenzi_separate; 
    int total_commands = 0;
public:
    Commandments(const char* cmd);
    ~Commandments();
    int GetTotalCMDs();
    char** char_convert(int nr_cmd);
    char* return_path(int nr_cmd);
};
