
#ifndef COBJPOOL_H_
#define COBJPOOL_H_
#include <set>
#include "CTType.h"
#include "Mutex.h"
#include <queue>
#include "exam_assert.h"
#include "glog_wrapper.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


#ifdef _MSC_VER
#pragma warning(disable:4251)
#endif

template<class T>
class CCObjPool
{
public:
	CCObjPool(  )
	{
		m_nMaxCount		= 0;
	}

	~CCObjPool(  )
	{
		Term() ;
	}

	CT_BOOL Init( CT_DWORD nInitCount, CT_DWORD nOnceExNums, CT_DWORD nReMainNums )
	{
		_AUTOMUTEX(&m_mutex);
		_EXAM_ASSERT( nInitCount > 0 );
		if ( nInitCount <= 0 )
			return CT_FALSE;
		
		m_nOnceExNums = nOnceExNums;
		m_nReMainNums = nReMainNums;

		ExpandOjb(nInitCount);
		
		return CT_TRUE;
	}

	CT_VOID ExpandOjb(CT_DWORD dwCount)
	{
		_AUTOMUTEX(&m_mutex);
		_EXAM_ASSERT( dwCount > 0 );
		for(CT_DWORD i = 0; i < dwCount; i++ )
		{
			try
			{
				T* p = new T;
				_EXAM_ASSERT(p != NULL);
				if (p != 0)
				{
					m_tQueue.push(p);
				}
			}
			catch (const std::bad_alloc& e)
			{
				LOG(ERROR) << "new message block bad_alloc: " << e.what();
			}
			catch (const std::exception& e)
			{
				LOG(ERROR) << "new message block exception: " << e.what();
			}
			catch (...)
			{
				LOG(ERROR) << "new message unkown exception";
			}
			
		}
		m_nMaxCount += dwCount;
	}

	CT_VOID Term( CT_VOID )
	{

		_AUTOMUTEX(&m_mutex);

		while(m_tQueue.size() != 0)
		{
			T* t = m_tQueue.front();
			delete t;
			m_tQueue.pop();
		}
		m_nMaxCount		= 0;
	}

	T* NewObj(  )
	{
		_AUTOMUTEX(&m_mutex);
		if (m_tQueue.size() <= m_nReMainNums)
		{
			if (m_nOnceExNums>0)
			{
				ExpandOjb(m_nOnceExNums);	
			}
			else
			{
				return NULL;
			}
		}

		T* t = m_tQueue.front();
		m_tQueue.pop();
		return t;
	}

	CT_VOID DeleteObj( T *pObj )
	{
		_AUTOMUTEX(&m_mutex);

		_EXAM_ASSERT( pObj != NULL );	
		if ( pObj == NULL )
		{
			return ;
		}

		m_tQueue.push(pObj);
	}

	CT_DWORD GetCount(  )
	{
		_AUTOMUTEX(&m_mutex);
		return m_tQueue.size();
	}

	CT_DWORD GetMaxCount()
	{
		_AUTOMUTEX(&m_mutex);
		return m_nMaxCount;
	}

private:
	typedef std::queue<T*>  TQueue;
	//obj
	TQueue					m_tQueue; 
	//
	CT_DWORD				m_nMaxCount;
	//
	CTMutex					m_mutex;
	//
	CT_DWORD				m_nOnceExNums;
	//
	CT_DWORD				m_nReMainNums;
};


#ifdef SOCKETS_NAMESPACE
}
#endif


#endif




