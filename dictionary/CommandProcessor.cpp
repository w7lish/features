#include "CommandProcessor.h"
#include "BLList.h"
#include "Util.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdio>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>        /* shmat(), IPC_RMID */
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

using namespace std;

// global variables
int msq_sem, shm_sem;  //semaphore IDs
char semaphoreName[1 + 6 + 1];

int g_msqkey;
int g_msqid;

void CommandProcessor::setup()
{
    static const char* szFunc{ "CommandProcessor::setup()" };
    std::cout << szFunc << std::endl;

	pid_t pid = getpid();

    // ftok to generate unique key 
    g_msqkey = ftok("/tmp", 66); 
// 	printf("\npid(%d):", pid);
//     cout << "mq key value:" << g_msqkey << endl;

    // msgget creates a message queue 
    // and returns identifier 
    g_msqid = msgget(g_msqkey, 0666 | IPC_CREAT); 
     if ( g_msqid < 0) {
	    printf("pid(%d):", pid);
        cout << "msgget error:" << errno << endl;
        exit(1);
    } else {
	    printf("pid(%d):", pid);
        printf("msgget: msgget succeeded: msqid = %d\n", g_msqid);
    }
}

void CommandProcessor::cleanup()
{
    static const char* szFunc{ "CommandProcessor::cleanup()" };
    std::cout << szFunc << std::endl;
	pid_t pid = getpid();

    // to destroy the message queue 
    printf("pid(%d): cleanup msg queue id %d\n", pid, g_msqid);
    msgctl(g_msqid, IPC_RMID, NULL); 
}

void CommandProcessor::run()
{
    static const char* szFunc{ "CommandProcessor::run()" };
 
	pid_t pid = getpid();
    std::cout << "(" << pid << ")-" << szFunc << std::endl;

	bool stop = false;

    MsgBuffer msg_buf;
    msg_buf.type = 1; 
  
    std::vector<string> tokens;
    std::vector<string> words;
    
    // fork a process to handle the request
    while ( !stop) {
        tokens.clear();
        string str; 

        // std::cout << "a\nb\nc\n";
        std::cout << "$";
        getline(cin, str); 
        std::cout << "$" << str << endl;

        const std::string s = ";"; 
        tokenize(str, s, tokens);
        
        // cout << "parsing result: tokens size =" << tokens.size() << endl;
        // for (auto &e: tokens) {
        //     cout << e << "," << endl;
        // }

        for (int i = 0; i < tokens.size(); i++) {
            std::string line = tokens[i];
            size_t idx = line.find_first_of(' ');
            
            std::string cmd;
            if (idx == std::string::npos) {
                cmd = line;
            } else {
                cmd = line.substr(0, idx);
            }
            trim(cmd);
            
            cout << "cmd:" << cmd << endl;
            if ( cmd.empty() || std::find(commands.begin(), commands.end(), cmd) == commands.end()) {
                cout << "invalid command" << endl;
                continue;
            }

            // reset text
            memset(msg_buf.msg.text, 0x00, MAX_TEXT_LEN);

            // map the command
            std::map<string, int>::const_iterator it = msg_ids.find(cmd);
            if ( it != msg_ids.end()) {
                msg_buf.msg.id = it->second;
                cout << "msg id:" << msg_buf.msg.id << endl;
            } else {
                cout << "command is not supported\n";
                continue;
            }

            // command list for debugging
            if ( cmd.compare("list") == 0 || cmd.compare("exit") == 0 ) {
                // string word = " ";
                // strncpy(msg_buf.msg.text, word.c_str(), word.length() > MAX_TEXT_LEN-1? MAX_TEXT_LEN-1: word.length() );
                send(pid, msg_buf);
                continue;
            }
            
            // words
            std::string word = line.substr(idx);
            trim(word);
            // std::string texts = line.substr(idx);
            // tokenize(texts, " ", words);
            if( cmd.compare("list") != 0 && word.empty() ) {
                cout << "invalid command" << endl;
                continue;
            }

            // sending 
            strncpy(msg_buf.msg.text, word.c_str(), word.length() > MAX_TEXT_LEN-1? MAX_TEXT_LEN-1: word.length() );
    		printf("\npid(%d)", pid);
    		printf("pusing command %s, id(%d), text(%s)\n", cmd.c_str(), msg_buf.msg.id, msg_buf.msg.text);

            // msgsnd to send message 
            send(pid, msg_buf);
            
            if ( cmd.compare("exit") == 0 ) {
                cout << "exiting ...\n";
                sleep(3);
                stop = true;
            }
        }
    } //while
    
    cleanup();
	printf("\npid(%d): ", pid);
    cout << "done resources cleaning up" << endl;
    signal(SIGTERM, SIG_DFL);
}

void CommandProcessor::send(pid_t pid, const MsgBuffer &msg_buf)
{
    struct sembuf asem[1];
    asem[0].sem_num = 0;
    asem[0].sem_op = 0;
    asem[0].sem_flg = 0;

    // push msg into the queue
    // P (mutex_sem);
    asem[0].sem_op = -1;
    if (semop ( msq_sem, asem, 1) == -1) {
		printf("pid(%d)", pid);
        cout << "semop/msq_sem p ***error:" <<  errno << ",exiting" << endl;
        exit(1);
    }

    // msgsnd to send message 
    if ( msgsnd(g_msqid, &msg_buf, sizeof(msg_buf), 0) < 0 ) {
		printf("pid(%d)", pid);
        cout << "msgsnd error:" <<  errno << endl;
        {
            // Release mutex sem: V (msg queue's mutex_sem)
            asem[0].sem_op = 1;
            if (semop (msq_sem, asem, 1) == -1) {
        		printf("\npid(%d)", pid);
                cout << "semop/msq_sem v error:" <<  errno << ", exiting..." << endl;
                exit(1);
            }
        }
    } else {
		printf("pid(%d)", pid);
		printf("sent id(%d), text(%s)\n", msg_buf.msg.id, msg_buf.msg.text);
    }
    
    // Release mutex sem: V (msg queue's mutex_sem)
    asem[0].sem_op = 1;
    if (semop (msq_sem, asem, 1) == -1) {
		printf("pid(%d)", pid);
        cout << "semop/msq_sem v error:" <<  errno << ", exiting..." << endl;
        exit(1);
    }
}