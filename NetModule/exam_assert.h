#ifndef		___DEF_EXAM_ASSERT_H_DEF___
#define		___DEF_EXAM_ASSERT_H_DEF___

#include "CTType.h"
#include <string>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

//enabled assert
#define _EXAM_ASSERT_TEST_

#ifdef _EXAM_ASSERT_TEST_  // 若使用断言测试

CT_VOID _exam_assert( std::string file_name, CT_INT32 line_no );

#define  _EXAM_ASSERT( condition ) \
if ( condition )\
	;\
else\
	_exam_assert( __FILE__, __LINE__ ) 

#else  // 若不使用断言测试

#define _EXAM_ASSERT(condition)

#endif  /* end of ASSERT */


#ifdef SOCKETS_NAMESPACE
}
#endif


#endif			//___DEF_EXAM_ASSERT_H_DEF___



















