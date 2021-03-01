#include "Thread.h"
#include <thread>
#include <iostream>

//
// TODO: Use shared_ptr for pThread_
//

bool Thread::init()
{
    std::cout << "Thread::init()" << std::endl;
    
	if ( !initialized_ ) {
		pThread_ = new std::thread(runExternal, this);
		if ( pThread_ ) {
			initialized_ = true;
			return true;
		}
	}
	return false;
}

bool Thread::uninit()
{
	if ( initialized_) {
		if( pThread_ ) {
		    if(pThread_->joinable()) {
			    pThread_->join();
		    }
		    
			delete pThread_;
			pThread_ = nullptr;
			initialized_ = false;
			return true;
		}
	}
	return false;
}

void Thread::runExternal(Thread* th)
{
	return th->run();
}
