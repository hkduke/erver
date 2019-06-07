/*! @file       ReadWriteBuffer.cpp
    @version    2.0
    @brief      Byte buffer management
*/

// local
#include "ReadWriteBuffer.h"
#include "exam_assert.h"
#include "glog_wrapper.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


///////////////////////////////////////////////////////////////////////////////
// ReadWriteBuffer

ReadWriteBuffer::ReadWriteBuffer()
    : mBufSiz(0)
    , mBufLen(0)
    , mBufIdx(0)
    , mGrowBy(SIZE_1K)
    , mIsExt(false)
    , mBuf(NULL)
{
	
}

ReadWriteBuffer::ReadWriteBuffer(
    const ReadWriteBuffer & rhs
    )
    : mBufSiz(0)
    , mBufLen(0)
    , mBufIdx(0)
    , mGrowBy(SIZE_1K)
    , mIsExt(false)
    , mBuf(NULL)
{
    operator=(rhs);
} 

ReadWriteBuffer & 
ReadWriteBuffer::operator=(
    const ReadWriteBuffer & rhs
    )
{
    Deallocate();
    if (rhs.mBufSiz > 0) {
        SetInternalBuffer(rhs.mBufSiz, rhs.mGrowBy ? rhs.mGrowBy : SIZE_1K);
        char * pBuf = GetWriteBuffer(rhs.mBufLen);
        memcpy(pBuf, rhs.mBuf, rhs.mBufLen);
        mBufIdx = rhs.mBufIdx;
        mBufLen = rhs.mBufLen;
    }
    return *this;
}

ReadWriteBuffer::~ReadWriteBuffer()
{
    Deallocate();
}

void ReadWriteBuffer::Deallocate()
{
    if (mBuf && !mIsExt) {
        free(mBuf);
    }

    mBufSiz = 0;
    mBufIdx = 0;
    mBufLen = 0;
    mGrowBy = SIZE_1K;
    mIsExt  = false;
    mBuf    = NULL;
}

void ReadWriteBuffer::Init()
{
	Deallocate();
	//设置缓冲大小
	SetInternalBuffer(SIZE_1K, SIZE_1K);
}

void ReadWriteBuffer::SetEmpty()
{
    mBufIdx = 0;
    mBufLen = 0;
}


//设置扩展缓冲大小
void ReadWriteBuffer::SetExternalBuffer(
    void *  a_pBuf, 
    size_t  a_nBufSiz, 
    size_t  a_nBufLen,
    size_t  a_nGrowBy
    )
{
    if (!a_pBuf)
	{
		_EXAM_ASSERT(CT_FALSE);
		LOG(ERROR) << "ReadWriteBuffer->a_pBuf";
		return;
	}
    if (a_nBufSiz < 1)
	{
		_EXAM_ASSERT(CT_FALSE);
		LOG(ERROR) << "ReadWriteBuffer->a_nBufSiz";
		return;
	}
    if (a_nBufLen > a_nBufSiz)
	{
		_EXAM_ASSERT(CT_FALSE);
		LOG(ERROR) << "ReadWriteBuffer->a_nBufLen";
		return;
	}

    Deallocate();

    mBufSiz = a_nBufSiz;
    mBufIdx = 0;
    mBufLen = a_nBufLen;
    mGrowBy = a_nGrowBy; 
    mIsExt  = true;
    mBuf    = (char *) a_pBuf;
}

/*! round a value up to the nearest multiple of the block 
    @param nValue   value to be rounded up
    @param nBlock   block size to round the value to
 */
static inline size_t roundup(size_t nValue, size_t nBlock) {
    if ((nValue % nBlock) != 0) {
        nValue = (nValue / nBlock) * nBlock + nBlock;
    }
    return nValue;
}

//设置缓冲大小
void ReadWriteBuffer::SetInternalBuffer(
    size_t a_nInitialSize, 
    size_t a_nGrowBy
    )
{
    if (a_nInitialSize < 1)
	{
		LOG(ERROR) << "ReadWriteBuffer->a_nInitialSize";
		return;
	}
    if (a_nGrowBy < 1)
	{
		LOG(ERROR) << "ReadWriteBuffer->a_nGrowBy";
		return;
	}

    Deallocate();

    // internal buffer
    a_nInitialSize = roundup(a_nInitialSize, a_nGrowBy);
    mBuf = (char *) malloc(a_nInitialSize);
	_EXAM_ASSERT(NULL!=mBuf);
    if (!mBuf)
	{
		LOG(ERROR) << "ReadWriteBuffer->a_pBuf";
		return;
	}
    mBufSiz = a_nInitialSize;
    mGrowBy = a_nGrowBy;
}


//获得写指针位置
char * ReadWriteBuffer::GetWriteBuffer(size_t  a_nMinBytes)
{
    if (mBufLen + a_nMinBytes > mBufSiz) {
        if (mIsExt) {
            if (0 == mGrowBy)
			{
				_EXAM_ASSERT(CT_FALSE);
				LOG(ERROR) << "ReadWriteBuffer->external buffer full.";
				return NULL;
			}

            size_t nNewSiz = mBufSiz + roundup(a_nMinBytes, mGrowBy);
            char * pBuf = (char *) malloc(nNewSiz);
            if (!pBuf)
			{
				_EXAM_ASSERT(CT_FALSE);
				LOG(ERROR) << "ReadWriteBuffer->a_pBuf";
				return NULL;
			}

            memcpy(pBuf, mBuf, mBufLen);
            mBuf    = pBuf;
            mBufSiz = nNewSiz;
            mIsExt  = false;
        }
        else {
            size_t nNewSiz = mBufSiz + roundup(a_nMinBytes, mGrowBy);
            char * pBuf = (char *) realloc(mBuf, nNewSiz);
            if (!pBuf)
			{
				_EXAM_ASSERT(CT_FALSE);
				LOG(ERROR) << "ReadWriteBuffer->a_pBuf";
				return NULL;
			}
            mBuf    = pBuf;
            mBufSiz = nNewSiz;
        }
    }
    return mBuf + mBufLen;//设置写指针位置.
}


/*


原型：extern void *realloc(void *mem_address, unsigned int newsize);
语法：指针名=（数据类型*）realloc（要改变内存大小的指针名，新的大小）。//新的大小一定要大于原来的大小不然的话会导致数据丢失！
头文件：#include <stdlib.h> 有些编译器需要#include <malloc.h>，在TC2.0中可以使用alloc.h头文件

功能：先判断当前的指针是否有足够的连续空间，如果有，扩大mem_address指向的地址，并且将mem_address返回，
	如果空间不够，先按照newsize指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域，
	而后释放原来mem_address所指内存区域，同时返回新分配的内存区域的首地址。即重新分配存储器块的地址。
　　返回值：如果重新分配成功则返回指向被分配内存的指针，否则返回空指针NULL。　
　　注意：这里原始内存中的数据还是保持不变的。当内存不再使用时，应使用free()函数将内存块释放。

  　1、如果有足够空间用于扩大mem_address指向的内存块，则分配额外内存，并返回mem_address。
   　　这里说的是“扩大”，我们知道，realloc是从堆上分配内存的，当扩大一块内存空间时， realloc()试图直接从堆上现存的数据后面的那些字节中获得附加的字节，如果能够满足，自然天下太平。也就是说，如果原先的内存大小后面还有足够的空闲空间用来分配，加上原来的空间大小= newsize。那么就ok。得到的是一块连续的内存。
	 　　2、如果原先的内存大小后面没有足够的空闲空间用来分配，那么从堆中另外找一块newsize大小的内存。
	   　　并把原来大小内存空间中的内容复制到newsize中。返回新的mem_address指针。（数据被移动了）。
		 　　老块被放回堆上。


realloc使用总结

1. realloc失败的时候，返回NULL
2. realloc失败的时候，原来的内存不改变，不会释放也不会移动
3. 假如原来的内存后面还有足够多剩余内存的话，realloc的内存=原来的内存+剩余内存,realloc还是返回原来内存的地址; 假如原来的内存后面没有足够多剩余内存的话，realloc将申请新的内存，然后把原来的内存数据拷贝到新内存里，原来的内存将被free掉,realloc返回新内存的地址
4. 如果size为0，效果等同于free()。这里需要注意的是只对指针本身进行释放，例如对二维指针**a，对a调用realloc时只会释放一维，使用时谨防内存泄露。
5. 传递给realloc的指针必须是先前通过malloc(), calloc(), 或realloc()分配的
6.传递给realloc的指针可以为空，等同于malloc。

*/


//提交写大小.
void ReadWriteBuffer::CommitWriteBytes(size_t  a_nBytes)
{
    if (mBufLen + a_nBytes > mBufSiz)
	{
		LOG(ERROR) << "ReadWriteBuffer->a_nBytes";
		return;
	}
    mBufLen += a_nBytes;
}
//获得写大小
size_t ReadWriteBuffer::GetWriteSize() const
{
    return mBufSiz - mBufLen;
}
//写数据
void ReadWriteBuffer::WriteBytes(
    const char *    a_pBuf, 
    size_t          a_nBufLen
    )
{
    char * pBuf = GetWriteBuffer(a_nBufLen);
    memcpy(pBuf, a_pBuf, a_nBufLen);
    CommitWriteBytes(a_nBufLen);
}
//获得读指针位置
const char * ReadWriteBuffer::GetReadBuffer() const
{
    return mBuf + mBufIdx;
}

//提交读大小
void ReadWriteBuffer::CommitReadBytes(
    size_t  a_nBytes
    )
{
    if (mBufIdx + a_nBytes > mBufLen)
	{
		LOG(ERROR) << "ReadWriteBuffer->a_nBytes";
		return;
	}
    mBufIdx += a_nBytes;
}

//获得读大小
size_t ReadWriteBuffer::GetReadSize() const
{
    return mBufLen - mBufIdx;
}

//读数据
void ReadWriteBuffer::ReadBytes(
    char * a_pBuf, 
    size_t a_nBufLen
    )
{
    if (GetReadSize() < a_nBufLen) {
		LOG(ERROR) << "ReadWriteBuffer->a_nBufLen";
		return;
    }

    const char * pBuf = GetReadBuffer();
    memcpy(a_pBuf, pBuf, a_nBufLen);
    CommitReadBytes(a_nBufLen);
}

//重置读写位置.
void ReadWriteBuffer::Compact()
{
    if (GetReadSize() > 0) {
        memmove(mBuf, GetReadBuffer(), GetReadSize());
    }

    mBufLen -= mBufIdx;
    mBufIdx  = 0;
}

bool ReadWriteBuffer::operator==(
    const ReadWriteBuffer & rhs
    ) const//对比缓冲数据一致性.
{
    if (GetReadSize() != rhs.GetReadSize()) {
        return false;
    }
    return 0 == memcmp(GetReadBuffer(), rhs.GetReadBuffer(), GetReadSize());
}




#ifdef SOCKETS_NAMESPACE
}
#endif











