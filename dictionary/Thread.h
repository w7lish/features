#pragma once

#include <thread>

class Thread
{
protected:
	std::thread	* pThread_;
	bool initialized_;

	virtual void run() = 0;
	
public:
	Thread() 
	    : initialized_(false), pThread_(nullptr) 
	{}

	virtual ~Thread() {}

	bool init();
	bool uninit();

	void static runExternal(Thread * thisThread );
};
