/* ----------------------------------------------------------------------------------------------------------


Created by Zhang Hui in 2009-3-9
-----------------------------------------------------------------------------------------------------------*/
#ifndef __Thread_h__
#define __Thread_h__

#define ThreadLogOut	Log_Msg
#define TLL_Debug		Log_Debug
#define TLL_Normal		Log_Normal
#define TLL_Failed		Log_Failed
#define TLL_Error		Log_Error
#define TLL_Warning		Log_Warning

enum eThreadRunStatus
{
	Trs_Init	= 0,
	Trs_Blocked	= 1,
	Trs_Running	= 2,
	Trs_Stopped	= 3, 
};

#if defined(_LINUX)	//Linux platform

#include <pthread.h>

void* ThreadFunction( void* pArgs );

class Thread
{
public:
	virtual int		ThreadInit() = 0;
	virtual void	Run() = 0;
	virtual int		CanBlocked() = 0;

	int				CreateThread();
	void			WakeUp();
	void			StopThread();

	Thread();
	virtual ~Thread();

protected:
	void			DetermineBlock();
  
	eThreadRunStatus			m_eRunStatus;

	pthread_t					m_hThread;
	pthread_attr_t				m_stAttr;
	pthread_mutex_t				m_stMutex;
	pthread_cond_t				m_stCond;

	char						m_szRetVal[64];
};

#else	//WIN32 or other platfrom

class Thread
{
public:
	virtual int		ThreadInit() = 0;
	virtual void	Run() = 0;
	virtual int		CanBlocked() = 0;

	int				CreateThread() { return 0; }
	void			WakeUp() {}
	void			StopThread() {}

	Thread() {}
	virtual ~Thread() {}

protected:
	void DetermineBlock() {}
};

#endif

#endif
