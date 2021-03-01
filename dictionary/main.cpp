/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, PHP, Ruby, 
C#, VB, Perl, Swift, Prolog, Javascript, Pascal, HTML, CSS, JS
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <iostream>
#include <sstream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <condition_variable>
#include <mutex>

// #define EXIT_FAILURE -1 
#define EXIT_SUCCESS 0

static void show_usage(std::string name)
{
	std::cerr << "Usage: " << name << " Options:\n"
			<< "\t-?,--help\t\t\tShow this help message"
// 			<< "\t-h,--host <ip>\t\t\tSpecify the host IP accepting the requests\n"
// 			<< "\t-p,--servicePort <n>\t\tSpecify the port accepting requests"
			<< std::endl;
}

// Set when process is terminated
static bool terminated = false;

// Used to block until terminated.
static std::mutex shutdown_mutex;
static std::condition_variable shutdown_cv;

// Blocks the main thread until sigHandler gets SIGTERM.
static void block_until_process_terminated()
{
	std::unique_lock<std::mutex> shut_lock(shutdown_mutex);

	// Wait for termination. Note: shut_lock is aquired and released while waiting, 
	shutdown_cv.wait(shut_lock, [](){ return terminated; } );
}

static void sigHandler(int signum, siginfo_t* info, void* ptr)
{
	static const char* szFunc{ "sigHandler()" };
	int status = 0;

	printf( "%s: Interrupt signal(%d) received from sender pid %lu\n"
			, szFunc, signum, (unsigned long)info->si_pid);

	// terminate program
	switch(signum) {
	    std::cout << "received sig:" << signum << std::endl;
	    
		case SIGTERM:
		case SIGABRT:
		{
			std::lock_guard<std::mutex> guard(shutdown_mutex);
			terminated = true;
			shutdown_cv.notify_all();
			break;
		}
		case SIGHUP:
		case SIGUSR1:
		default:
			break;
	}
}

#include "DictServer.h"

int main ( int argc, char * argv[] )
{
	static const char* szFunc{ "main()" };

	try
	{
		for (int i = 1; i < argc; ++i)
		{
			bool bUsageError = true;
			std::string arg = argv[i];

			if ((arg == "-?") || (arg == "--help"))
			{
				 show_usage(argv[0]);
				 return 0;
			}

			// Did we have a usage error parsing this option
			if (bUsageError)
			{
				show_usage(argv[0]);
				return EXIT_FAILURE;
			}
		} 
	}
	catch(std::exception &ex)
	{
		std::cout << "Caught exception on parsing command line:" << ex.what() << std::endl;
		show_usage(argv[0]);
		return EXIT_FAILURE;
	}

	{

	    // set sigal handler 
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGINT);

		struct sigaction act;        
		memset(&act, 0, sizeof(act));        
		act.sa_sigaction = sigHandler;
		act.sa_flags = SA_SIGINFO;
		act.sa_mask = set;
		
		//list of signals will caused termination
		if (sigaction(SIGINT, &act, NULL) != 0) {
			printf( "%s: SIGINT error %d\n", szFunc, errno);
		}
		
		if (sigaction(SIGTERM, &act, NULL) != 0) {
			printf( "%s: SIGTERM error %d\n", szFunc, errno);
		}

		if (sigaction(SIGABRT, &act, NULL) != 0) {
			printf( "%s: SIGTERM error %d\n", szFunc, errno);
		}
	}

    printf("%s: starting service...\n", szFunc);
    auto server = std::make_unique<DictServer>(DictServer());

	if( nullptr == server.get() ||  !server->start() ) {
		printf("%s: ******failed to start server, exiting...******\n", szFunc);
		return EXIT_FAILURE;
	}

	// ========================================================
	// Main thread is blocked here waiting for SIGTERM.
	printf("main thread blocking until terminated...\n");
	block_until_process_terminated();

	printf("%s: stopping service...\n", szFunc);
	bool stopped = server->stop();

	printf( "%s: ******exiting...******\n", szFunc);
	return stopped?EXIT_SUCCESS:EXIT_FAILURE;
}
