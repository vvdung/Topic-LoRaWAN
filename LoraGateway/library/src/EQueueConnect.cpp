#include "../inc/HFactor.h"
#include "../inc/EQueueConnect.h"

CEQueueConnect::CEQueueConnect(size_t maxQueue){
    m_maxQueue = maxQueue;
}

CEQueueConnect::~CEQueueConnect(){
    m_mutex.lock();
    while (!m_queue.empty())
    {
        CEConnect* p = m_queue.front();
        m_queue.pop();
        delete p;
    }    
    m_mutex.unlock();
}

void CEQueueConnect::Lock(){
    m_mutex.lock();
}
void CEQueueConnect::Unlock(){
    m_mutex.unlock();    
}
bool CEQueueConnect::TryLock(){
    return m_mutex.try_lock();
}

void CEQueueConnect::Push(CEConnect* p){
    if (!p) return;
    if (m_maxQueue <= 0 || m_queue.size() < m_maxQueue){
        m_queue.push(p);
        return;
    }

    CEConnect* q = m_queue.front();
    m_queue.pop();
    m_queue.push(p);
    delete q;
}

CEConnect* CEQueueConnect::Pop(){
    if (m_queue.empty()) return NULL;
    CEConnect* p = m_queue.front();
    m_queue.pop();
    return p;
}
size_t CEQueueConnect::Size(){
    return m_queue.size();
}