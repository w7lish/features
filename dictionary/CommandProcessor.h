#pragma once

#include "Thread.h"
#include <iostream>

struct MsgBuffer;
class CommandProcessor : public Thread
{
public:
	CommandProcessor() : Thread() {
	    setup();
	}
	
	virtual ~CommandProcessor() {}

protected:
	virtual void run ();
	void setup();
	void cleanup();
	
	void send(pid_t, const MsgBuffer&);
};
