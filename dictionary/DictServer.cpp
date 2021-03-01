#include "DictServer.h"
#include <iostream>

using namespace std;

bool DictServer::start()
{
    std::cout << "DictServer::start()" << std::endl;
    wproc_.init();
    cproc_.init();
    
    return true;
}

bool DictServer::stop()
{
    std::cout << "DictServer::stop()" << std::endl;
    wproc_.uninit();
    cproc_.uninit();
    return true;
}

