/* ----------------------------------------------------------------------------------------------------------


 in 2009-3-9
-----------------------------------------------------------------------------------------------------------*/
#include "Thread.h"
#include "Log.h"

#if defined(_LINUX)	//Linux platform

/*---------------------------------------------------------------------------------------------------------*/
void* ThreadFunction( void *pArgs )
{
	if( !pArgs )
	{
		return 0;
	}

	Thread *pThread = (Thread *)pArgs;

	if( pThread->ThreadInit() != 0 )
	{
		return 0;
	}

	pThread->Run();

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
Thread::Thread()
{
	m_eRunStatus = Trs_Init;
}

/*---------------------------------------------------------------------------------------------------------*/
Thread::~Thread()
{
}

/*---------------------------------------------------------------------------------------------------------*/
int Thread::CreateThread()
{
	pthread_attr_init( &m_stAttr );
	pthread_attr_setscope( &m_stAttr, PTHREAD_SCOPE_SYSTEM );
	pthread_attr_setdetachstate( &m_stAttr, PTHREAD_CREATE_JOINABLE );
	pthread_cond_init( &m_stCond, NULL );
	pthread_mutex_init( &m_stMutex, NULL );
	m_eRunStatus = Trs_Running;

	pthread_create( &m_hThread, &m_stAttr, ThreadFunction, (void *)this );

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void Thread::StopThread()
{
	pthread_mutex_lock( &m_stMutex );

	m_eRunStatus = Trs_Stopped;
	pthread_cond_signal( &m_stCond );

	pthread_mutex_unlock( &m_stMutex );

	//等待该线程终止
	pthread_join( m_hThread, NULL );
	ThreadLogOut( TLL_Normal, "Thread stopped.\n" );
}

/*---------------------------------------------------------------------------------------------------------*/
void Thread::WakeUp()
{
	pthread_mutex_lock( &m_stMutex );

	if( m_eRunStatus == Trs_Blocked && !CanBlocked() )
	{
		pthread_cond_signal( &m_stCond );
	}

	pthread_mutex_unlock( &m_stMutex );
}

/*---------------------------------------------------------------------------------------------------------*/
void Thread::DetermineBlock()
{
	pthread_mutex_lock( &m_stMutex );

	while( CanBlocked() || m_eRunStatus == Trs_Stopped )
	{
		if( m_eRunStatus == Trs_Stopped )
		{
			pthread_exit( ( void* )m_szRetVal );
		}
		m_eRunStatus = Trs_Blocked;
		pthread_cond_wait( &m_stCond, &m_stMutex );
	}

	m_eRunStatus = Trs_Running;

	pthread_mutex_unlock( &m_stMutex );
}

#endif

/* ----------------------------------------------------------------------------------------------------------
End of file */

