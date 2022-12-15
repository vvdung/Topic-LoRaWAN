#pragma once
#include <pthread.h>
#include <stdint.h>
#include <mutex>
#include <sys/time.h>

class CEThread
{
public:
	void			Start(uint32_t milliseconds = 100);
	void			Exit();
	void			NextDelay(uint32_t milliseconds = 100);
    void            WaitCompleted();
	pthread_t		m_uThreadID;

protected:
	CEThread(void);
	virtual ~CEThread(void);
	virtual void	InitInstance() = 0;
	virtual void	Run() = 0;
	virtual void	ExitInstance() = 0;
	static void*	_ThreadEpollRoutine(void* lpData);
	void			OnThreadEpollRoutine();
	uint32_t		m_dwDelay;
	bool			m_done;	

};
