#pragma once
#include "EConnect.h"

class CEQueueConnect{
public:
    CEQueueConnect(size_t maxQueue = 0);
    ~CEQueueConnect();
    void        Lock();
    void        Unlock();
    bool        TryLock();
    void        Push(CEConnect* p);
    CEConnect*  Pop();
    size_t      Size();
protected:
    std::queue<CEConnect*>  m_queue;
    std::mutex              m_mutex;
    size_t                  m_maxQueue;
};