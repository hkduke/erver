
#include "exam_assert.h"


#ifdef _EXAM_ASSERT_TEST_  // 若使用断言测试
#include <assert.h>
#include <cassert>
#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>


CT_VOID _exam_assert(std::string file_name, CT_INT32 line_no )
{
	printf( "\n[EXAM]Assert failed: %s, line %u\n",
		file_name.c_str(), line_no );
	abort();
}


#endif
























