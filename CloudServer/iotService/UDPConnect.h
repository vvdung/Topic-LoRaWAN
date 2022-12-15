#pragma once
#include "../inc/EConnect.h"

class CUDPConnect : public CEConnect{
public:
    friend class CUDPServer;
    CUDPConnect();
    virtual ~CUDPConnect();

protected:
    virtual void	OnCreated();
    virtual void    OnConnected();
	virtual void	OnReceived(uint8_t* pBuffer,int uSize);
	virtual void	OnSend(int uSize);
	virtual void	OnClosed();
    virtual void	OnTimerRoutine();

    uint32_t        m_uCntPING;//must have, server not auto close udp socket
};