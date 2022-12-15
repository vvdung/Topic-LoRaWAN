#pragma once
#include "../inc/EServer.h"
#include "UDPConnect.h"

class CUDPServer : public CEServer{
public:
    CUDPServer();
    ~CUDPServer();
protected:
    virtual bool    OnInit();
    virtual bool    OnExit();
    virtual void    OnAccept();
    virtual void    OnDisconnect(CEConnect* p);
    virtual void	OnCreated();    
    
    CUDPConnect*    CreateConnect(struct sockaddr_in* pClientAddr);
};