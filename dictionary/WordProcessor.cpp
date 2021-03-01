#include "WordProcessor.h"
#include "BLList.h"

#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <climits>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

extern BlockList* glist;
#define MSQ_SEM_MUTEX_KEY "/tmp/."  // message queue mutex semaphore 
#define SHM_SEM_MUTEX_KEY "/."  // shared memory mutex semaphore
#define SHM_KEY "/var/."  // shared memory mutex semaphore

extern int g_msqkey;
extern int g_msqid;

void WordProcessor::setup()
{
    static const char* szFunc{ "WordProcessor::setup()" };
    std::cout << szFunc << std::endl;
    
    msq_sem = createSem(MSQ_SEM_MUTEX_KEY);
    shm_sem = createSem(SHM_SEM_MUTEX_KEY);
    createShm(SHM_SEM_MUTEX_KEY);
}

int WordProcessor::createSem(const std::string& keyname)
{
    static const char* szFunc{ "WordProcessor::createSm()" };
    // std::cout << szFunc << std::endl;
	pid_t pid = getpid();

    // create semaphore
    key_t sem_key;
    int sem_id;
    union semun  
    {
        int val;
        struct semid_ds *buf;
        ushort array [1];
    } sem_attr;
    
    // create mutex semaphore
    
    // key
    if ((sem_key = ftok (keyname.c_str(), 66)) == -1) {
	    printf("pid(%d):", pid);
        perror("ftok");
        exit(1);
    }
    
    // id
    if ((sem_id = semget (sem_key, 1, 0660 | IPC_CREAT)) == -1) {
	    printf("pid(%d):", pid);
        perror("semget");
        exit(1);
    } else {
	    printf("pid(%d):", pid);
        printf("semget: created semaphore id %d\n", sem_id);
    }
    
    // Giving initial value. 
    //  mutual exclusion semaphore, mutex_sem with an initial value 1.
    sem_attr.val = 1;        // unlocked
    if (semctl (sem_id, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); 
        exit (1);
    } else {
        ;
	   // printf("pid(%d):", pid);
        // printf("semctl: msq_sem sem_attr.val = %d \n", sem_attr.val);
    }
    
    return sem_id;
}

void WordProcessor::createShm(const std::string& keyname)
{
    static const char* szFunc{ "WordProcessor::createSem()" };
    std::cout << szFunc << std::endl;
	pid_t pid = getpid();

    //
    // *** shared memory
    //
    // ftok to generate unique key 
    key_t key_s = ftok(keyname.c_str(), 65); 
// 	printf("pid(%d):", pid);
//     cout << "key_s:" << key_s << endl;
  
    // shmget returns an identifier in shmid 
    int shmid = shmget(key_s, sizeof(BlockList), 0666|IPC_CREAT); 
    if ( shmid < 0) {
		printf("pid(%d):", pid);
        cout << "shmget error:" << errno << endl;
        exit(1);
    }

    // attach 
    glist = (BlockList*) shmat(shmid, NULL, 0);
    if ( glist == (void *) (-1)) {
    	printf("pid(%d):", pid);
        cout << "shmat error:" << errno << endl;
        exit(1);
    }
    
    // init glist
    glist->head = DNULL;
    glist->pfree = DNULL;
    glist->npool = 0;

// 	printf("pid(%d):", pid);
//     cout << "size of BlockList:" << sizeof(BlockList) << endl;
}

void WordProcessor::cleanup()
{
	pid_t pid = getpid();
    // Remove semaphores
    if (semctl (msq_sem, 0, IPC_RMID) == -1) {
	    printf("pid(%d):", pid);
        cout << "semctl IPC_RMID error:" << errno << endl;
    }

    if (semctl (shm_sem, 0, IPC_RMID) == -1) {
	    printf("pid(%d):", pid);
        cout << "semctl IPC_RMID error:" << errno << endl;
    }
}

bool WordProcessor::init()
{
	static const char* szFunc{ "WordProcessor::init()" };
    std::cout << szFunc << std::endl;
    bool ret = false;
    
    // fork two processes to handle the request from the queue
	pid_t pid = getpid();
    // cout << "Parent pid:" << pid << endl;
    
	pid_t pid1 = fork();
	sleep(3);
	pid_t pid2 = fork();
	if( pid1 > 0 && pid2 > 0 ) {  // Parent
// 		printf("parent waits...\n" );
		wait(NULL);
		wait(NULL); 
		
		sleep(1);
		ret = true;

		return ret;
		
	} else if ( pid1 ==0 && pid2 > 0 ) {
        // cout << "WordProcessor -- pid1" << endl;

		// wait for sockets being actually closed
		sleep(2);

        // TODO do actual work here
        process();
        
		// should never reached, this indicates failure
		exit(EXIT_FAILURE);
	} else if ( pid2 ==0 && pid1 > 0 ) {
        // cout << "WordProcessor -- pid2" << endl;

		// wait for sockets being actually closed
		sleep(5);

        // TODO do actual work here
        process();
        
		// should never reached, this indicates failure
		exit(EXIT_FAILURE);
	}
	
	return ret;
}

void WordProcessor::process()
{
	static const char* szFunc{ "WordProcessor::process()" };
	volatile bool stop = false;
	
	pid_t pid = getpid();
    std::cout << "(" << pid << ")-" << szFunc << std::endl;

    struct sembuf asem[2];
    asem[0].sem_num = 0;
    asem[0].sem_op = 0;
    asem[0].sem_flg = 0;

    // message buffer
    MsgBuffer msg_buf;
// 	printf("\npid(%d), msqid(%d)\n", pid, g_msqid);

    int i = 0;
    while ( !stop ) {
        // for debugging
        if ( i > INT_MAX-1) i == 0;
        if ( i++%599 == 0) {
		    printf("pid(%d): waiting for request...\n", pid);
        }
        sleep(1);

        // pull from msg queue
        // P (mutex_sem);
        asem[0].sem_op = -1;
        if (semop ( msq_sem, asem, 1) == -1) {
    		printf("pid(%d)", pid);
            cout << "semop/msq_sem p ***error:" <<  errno << ",exiting" << endl;
            exit(2);
        }

        // msgrcv to receive message
        if (msgrcv(g_msqid, &msg_buf, sizeof(msg_buf), 0, 0|IPC_NOWAIT) < 0) {
		    if ( errno == ENOMSG ) {
		        // V option
                asem[0].sem_op = 1;
                if (semop (msq_sem, asem, 1) == -1) {
            		printf("pid(%d)", pid);
                    cout << "semop/msq_sem v error:" <<  errno << ", exiting..." << endl;
                    exit(1);
                }
		        continue;
		    } else { 
        		printf("pid(%d):", pid);
    		    cout << "msgrcv errno:" << errno << endl;
    		    exit(2);
		    }
	    } else {
            // display the message 
    		printf("pid(%d):", pid);
            printf(" msqid(%d) received - %d:%s\n", g_msqid, msg_buf.msg.id, msg_buf.msg.text); 
	    }

         // Release mutex sem: V (msg queue's mutex_sem)
        asem[0].sem_op = 1;
        if (semop (msq_sem, asem, 1) == -1) {
    		printf("pid(%d)", pid);
            cout << "semop/msq_sem v error:" <<  errno << ", exiting..." << endl;
            exit(1);
        }
            
        // database requests
        asem[0].sem_op = -1;
        if (semop ( shm_sem, asem, 1) == -1) {
    		printf("pid(%d)", pid);
            cout << "semop/shm_sem p error:" <<  errno << endl;
            exit(2);
        } 
// 		printf("\npid(%d)", pid);
//         cout << "processing from database: " << msg_buf.msg.id << ", " << msg_buf.msg.text << endl;

        Node *node = NULL;
        switch (msg_buf.msg.id) {
            case 0: // insert
            {
                node = node_push(msg_buf.msg.text);
        		printf("pid(%d): done insertion %s\n", pid, msg_buf.msg.text);
            }
            break;
            case 1: // delete
            {
                node_delete(msg_buf.msg.text);
        		printf("pid(%d): done deletion %s\n", pid, msg_buf.msg.text);
            }
            break;
            case 2: // search
            {
                node = node_search(msg_buf.msg.text);
        		printf("pid(%d): done search %s\n", pid, msg_buf.msg.text);
            }
            break;
            case 3: // list
        	{
        	    printf("pid(%d):\n", pid);
        		node_list();
        	}
            break;
            case 4: // exit
        	{
        	    printf("pid(%d): exiting...\n", pid);
        		stop = true;
        	}
            break;
            default:
        		printf("pid(%d): invalid command %d\n", pid, msg_buf.msg.id);
            break;
        }
        
        // Release mutex sem: V (shared memory's mutex_sem)
        asem[0].sem_op = 1;
        if (semop (shm_sem, asem, 1) == -1) {
    		printf("pid(%d)", pid);
            cout << "semop/shm_sem v error:" <<  errno << endl;
            exit(2);
        } else {
    // 		printf("\npid(%d)", pid);
    //         cout << "semop/shm_sem v succeeds:" <<  errno << endl;
        }
    } // while
    
    //detach from shared memory  
    shmdt(glist); 
    
    // destroy the shared memory 
    shmctl(shm_sem, IPC_RMID, NULL); 

    // to destroy the message queue 
    // msgctl(g_msqid, IPC_RMID, NULL); 
    
    // cleanup 
    cleanup();
    
	printf("pid(%d): ", pid);
    cout << "done resources cleaning up" << endl;
}
