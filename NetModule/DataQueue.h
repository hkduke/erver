#ifndef CCDATAQUEUE_H_
#define CCDATAQUEUE_H_
#include <queue>
#include "CTType.h"
#include "Mutex.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


#ifdef _MSC_VER
#pragma warning(disable:4251)
#endif

//消息队列
template<typename T>
class CDataQueue
{
public:
	CDataQueue()
	{
		
	}

	~CDataQueue()
	{
		_AUTOMUTEX(&m_mutex);
		if (!m_Queue.empty())
		{
			m_Queue.pop();
		}
	}
public:
	//插入数据
	CT_VOID PushBack(T block)
	{
		_AUTOMUTEX(&m_mutex);
		m_Queue.push(block);
	}
	//得到头
	T GetHead(CT_VOID)
	{
		_AUTOMUTEX(&m_mutex);
		if (!m_Queue.empty())
		{
			T p = m_Queue.front();
			m_Queue.pop();
			return p;
		}
		return NULL;
	}
	//得到数据长度
	CT_DWORD GetSize(CT_VOID)
	{
		_AUTOMUTEX(&m_mutex);
		return (CT_DWORD)m_Queue.size();
	}
private:
	typedef std::queue<T> DataQueue;
	//数据队列
	DataQueue	m_Queue;
	//互斥锁
	CTMutex		m_mutex;
};


#ifdef SOCKETS_NAMESPACE
}
#endif


#endif





