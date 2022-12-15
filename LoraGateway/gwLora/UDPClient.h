#pragma once
#include "../inc/EClient.h"

class CUDPClient : public CEClient{
public:
    CUDPClient();
    virtual ~CUDPClient();
    
protected:
    virtual void	OnCreated();
    virtual void    OnConnected();
    virtual void	OnReceived(uint8_t* pBuffer,int uSize);
	virtual void	OnSend(int uSize);
	virtual void	OnClosed();
    virtual void	OnTimerRoutine();
        
};