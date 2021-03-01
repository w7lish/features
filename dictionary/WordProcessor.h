#pragma once

#include <iostream>
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */

#define WAIT_INTERVAL   1
#define MAX_WAIT_CNT    30

class WordProcessor 
{
public:
	WordProcessor() {
	    setup();
	}

	~WordProcessor() {
	   // cleanup();
	}
	
	bool init();
	bool uninit() { return true;} // TODO

private:
    void setup();
    void cleanup();
    
    // create semaphores
    int createSem(const std::string& keyname);
    // create shared memory
    void createShm(const std::string& keyname);
    
	void process();

    int msq_sem = 0;
    int shm_sem = 0;

};
