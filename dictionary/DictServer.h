#include "CommandProcessor.h"
#include "WordProcessor.h"

class DictServer
{
public:
	DictServer() {}

	~DictServer() {}

	bool start();
	bool stop();
	
private:

	CommandProcessor cproc_;
	WordProcessor wproc_;
	
};
