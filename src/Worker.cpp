#include <iostream>
#include <unistd.h>
#include "Worker.h"
#include "Sunnet.h"
using namespace std;

void Worker::operator()() {
    while(true) {
        shared_ptr<Service> srv = Sunnet::inst->PopGlobalQueue();
        if (!srv)
        {
            Sunnet::inst->WorkerWait();
        }else{
            srv->ProcessMsgs(eachNum);
            CheckAndPutGlobal(srv);
        }
        
    }
}

void Worker::CheckAndPutGlobal(shared_ptr<Service> srv){
    if (srv->isExiting)
    {
        return;
    }
    pthread_mutex_lock(&srv->queueLock);
    {
        if (!srv->msgQueue.empty())
        {
            Sunnet::inst->PushGlobalQueue(srv);
        }else{
            srv->SetInGlobal(false);
        }
    }
    pthread_mutex_unlock(&srv->queueLock);
}