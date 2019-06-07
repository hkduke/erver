
#ifndef _CEGUISingleton_h_
#define _CEGUISingleton_h_
#include "exam_assert.h"

#ifdef _WIN32
#pragma warning(disable:4661)
#endif



#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


template <typename T> 
class  CCSingleton
{
protected:
    static  T* ms_Singleton;

public:
    CCSingleton()
    {
        _EXAM_ASSERT( !ms_Singleton );
        ms_Singleton = static_cast<T*>(this);
    }
   ~CCSingleton(  )
        {  _EXAM_ASSERT( ms_Singleton );  ms_Singleton = 0;  }
    static T& getSingleton( void )
        {  _EXAM_ASSERT( ms_Singleton );  return ( *ms_Singleton );  }
    static T* getSingletonPtr( void )
        {  return ( ms_Singleton );  }

private:
    CCSingleton& operator=(const CCSingleton&) { return *this; }
    CCSingleton(const CCSingleton&) {}
};

//macOs没有这句会报很多警告
#ifdef _OS_MAC_CODE
template<class T>
T* CCSingleton<T>::ms_Singleton = CCSingleton<T>::getSingletonPtr();
#endif // _OS_MAC_CODE

#ifdef SOCKETS_NAMESPACE
}
#endif


#endif	






