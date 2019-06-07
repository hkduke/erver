
#ifndef CCMEMPOOL_H_
#define CCMEMPOOL_H_
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "Mutex.h"
#include "Singleton.h"
#include "exam_assert.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

#ifdef _WIN32
#pragma warning(disable:4251)
#endif

union _Obj { union _Obj* _M_free_list_link;  char _M_client_data[1]; };

class CCMemPool : public CCSingleton<CCMemPool>
{
public:
	CCMemPool();
	~CCMemPool();
private:
	CTMutex m_mutex;
    enum {_ALIGN = 8};
    enum {_MAX_BYTES = 512};
    enum {_NFREELISTS = 64}; 
	 _Obj*  _S_free_list[_NFREELISTS]; 
	 char* _S_start_free;
	 char* _S_end_free;
	 size_t _S_heap_size;

	 size_t  _S_round_up(size_t __bytes) 
	 { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }


	 size_t _S_freelist_index(size_t __bytes)
	 {
		return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
	 }
	 void* _S_refill(size_t __n);
	 char* _S_chunk_alloc(size_t __size, int& __nobjs);

	 void*   reallocate(void* __p,
		 size_t __old_sz,
		 size_t __new_sz);

public:
	void Deallocate(void* __p, size_t __n);
	void* Allocate(size_t __n);
};



#ifdef SOCKETS_NAMESPACE
}
#endif

#endif





