#pragma once
#include <queue>
#include <thread>
#include "Msg.h"
using namespace std;

class Service {
public:
    // 唯一id
    uint32_t id;
    // 类型
    shared_ptr<string> type;
    // 是否正在退出
    bool isExiting = false;
    // 消息列表和锁
    queue<shared_ptr<BaseMsg>> msgQueue;
    pthread_mutex_t queueLock;
    // 标记是否在全局队列
    bool inGlobal = false;
    pthread_mutex_t inGlobalLock;

public:
    Service();
    ~Service();
    // 回调函数
    void OnInit();
    void OnMsg(shared_ptr<BaseMsg> msg);
    void OnExit();
    // 插入消息
    void PushMsg(shared_ptr<BaseMsg> msg);
    // 执行消息
    bool ProcessMsg();
    void ProcessMsgs(int max);
    void SetInGlobal(bool isIn);
private:
    // 取出一条消息
    shared_ptr<BaseMsg> PopMsg();
    // 消息处理方法
    void OnServiceMsg(shared_ptr<ServiceMsg> msg);
    void OnAcceptMsg(shared_ptr<SocketAcceptMsg> msg);
    void OnRWMsg(shared_ptr<SocketRWMsg> msg);
    void OnSocketData(int fd, const char* buff, int len);
    void OnSocketWritable(int fd);
    void OnSocketClose(int fd);
};