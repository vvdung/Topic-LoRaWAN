#pragma once

#include "../inc/EClient.h"


class CMQTTClient : public CEClient
{
public:
    CMQTTClient();
    virtual ~CMQTTClient();
    
	bool ConnectEx(const char *lpszHostAddress, uint32_t nHostPort, uint8_t* pData = NULL, uint32_t uSize = 0);
protected:
    virtual void	OnCreated();
    virtual void    OnConnected();
    virtual void	OnReceived(uint8_t* pBuffer,int uSize);
	virtual void	OnSend(uint8_t *pBuffer,int uSize);
	virtual void	OnClosed();
    virtual void	OnTimerRoutine();
    
    CByteBuffer*    m_pConnectedBytes;
};