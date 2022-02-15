#include "Service.h"
#include "Sunnet.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

Service::Service(){
    // 初始化锁
    pthread_mutex_init(&queueLock, NULL);
    pthread_mutex_init(&inGlobalLock, NULL);
}

Service::~Service(){
    pthread_mutex_destroy(&queueLock);
    pthread_mutex_destroy(&inGlobalLock);
}

void Service::PushMsg(shared_ptr<BaseMsg> msg){
    pthread_mutex_lock(&queueLock);
    {
        msgQueue.push(msg);
    }
    pthread_mutex_unlock(&queueLock);
}

shared_ptr<BaseMsg> Service::PopMsg(){
    shared_ptr<BaseMsg> msg = NULL;
    // 取一条消息
    pthread_mutex_lock(&queueLock);
    {
        if (!msgQueue.empty())
        {
            msg = msgQueue.front();
            msgQueue.pop();
        }
        
    }
    pthread_mutex_unlock(&queueLock);
    return msg;
}

void Service::OnInit() {
    cout << "[" << id << "] OnInit" << endl;
    // 开启监听
    Sunnet::inst->Listen(8002, id);
}

void Service::OnMsg(shared_ptr<BaseMsg> msg){
    // SERVICE
    if (msg->type == BaseMsg::TYPE::SERVICE)
    {
        auto m = dynamic_pointer_cast<ServiceMsg>(msg);
        OnServiceMsg(m);
    }
    // SOCKET_ACCEPT
    if (msg->type == BaseMsg::TYPE::SOCKET_ACCEPT)
    {
        auto m = dynamic_pointer_cast<SocketAcceptMsg>(msg);
        OnAcceptMsg(m);
    }
    // SOCKET_RW
    if (msg->type == BaseMsg::TYPE::SOCKET_RW)
    {
        auto m = dynamic_pointer_cast<SocketRWMsg>(msg);
        OnRWMsg(m);
    }
}

void Service::OnExit()
{
    cout << "[" << id << "] OnExit" << endl;
}

bool Service::ProcessMsg() {
    shared_ptr<BaseMsg> msg = PopMsg();
    if(msg){
        OnMsg(msg);
        return true;
    }else{
        return false;
    }
}

void Service::ProcessMsgs(int max){
    for (int i = 0; i < max; i++)
    {
        bool succ = ProcessMsg();
        if (!succ)
        {
            break;
        }
    }
}

void Service::SetInGlobal(bool isIn){
    pthread_mutex_lock(&inGlobalLock);
    {
        inGlobal = isIn;
    }
    pthread_mutex_unlock(&inGlobalLock);
}

void Service::OnServiceMsg(shared_ptr<ServiceMsg> msg){
    cout << "OnServiceMsg " << endl;
}

void Service::OnAcceptMsg(shared_ptr<SocketAcceptMsg> msg){
    cout << "OnAcceptMsg " << msg->clientFd << endl;
}

void Service::OnRWMsg(shared_ptr<SocketRWMsg> msg){
    int fd = msg->fd;
    // 可读
    if (msg->isRead)
    {
        const int BUFFSIZE = 512;
        char buff[BUFFSIZE];
        int len = 0;
        do
        {
            len = read(fd, &buff, BUFFSIZE);
            if (len > 0)
            {
                OnSocketData(fd, buff, len);
            }
        } while (len == BUFFSIZE);
        
        if (len <= 0 && errno != EAGAIN)
        {
            if (Sunnet::inst->GetConn(fd))
            {
                OnSocketClose(fd);
                Sunnet::inst->CloseConn(fd);
            }
        }
    }
    // 可写
    if (msg->isWrite)
    {
        if (Sunnet::inst->GetConn(fd))
        {
            OnSocketWritable(fd);
        }
    }
}

void Service::OnSocketData(int fd, const char* buff, int len){
    cout << "OnSocketData" << fd << " buff: " << buff << endl;
    
    // char writeBuff[3] = {'l', 'p', 'y'};
    write(fd, buff, len);
}

void Service::OnSocketWritable(int fd) {
    cout << "OnSocketWritable " << fd << endl;
}

void Service::OnSocketClose(int fd) {
    cout << "OnSocketClose " << fd << endl;
}