#include "stdafx.h"
#include "Random.h"
//#include <wincrypt.h>
#include <random>
unsigned int Randfun::m_dwSeed = 0;
//HCRYPTPROV  Randfun::m_hProvider = 0;

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/time.h>

std::random_device g_rd;
std::mt19937 g_gen(g_rd());

Randfun::Randfun()
{
	
}


Randfun::~Randfun()
{
	
}
// �������������
void Randfun::SetRandomSeed(unsigned int dwSeed)
{
	m_dwSeed = dwSeed;
}

// �������չ,Ч�ʸ���
long long  Randfun::GetRandom(long long  llMax, long long  llMin)
{
	if (llMin >= llMax) return llMin;

	/*if (llMin >= llMax) return llMin;
	return GetRealRandom(llMax, llMin);*/

#ifdef _WIN32
	/*std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(llMin,llMax);
	long long  nRandRes = dis(gen);
	return nRandRes;*/
#else
 	/*int rand_int = 0;
	int fd;
	fd = open ("/dev/random", O_RDONLY);
	if (fd > 0)
	{
		read(fd,&rand_int,sizeof(int));
	}
	close(fd);
    long long nRandRes = (rand_int % (llMax - llMin)) + llMin;
    return nRandRes;*/

 	/*static bool b = false;
	struct timeval tv;
	if(b == false)
	{
		gettimeofday(&tv, NULL);
		srand(tv.tv_usec);
		b = true;
	}
	long long nRandRes = (rand() % (llMax - llMin)) + llMin;
	return nRandRes;*/
#endif

	std::uniform_int_distribution<> dis(llMin,llMax);
	long long  nRandRes = dis(g_gen);
	return nRandRes;

}
bool Randfun::InitRealRandom()
{
	/*if (!::CryptAcquireContextW(&m_hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
	{
		::CryptReleaseContext(m_hProvider, 0);
		return false;
	}*/
	return true;
}
void Randfun::ReleaseRealRandom()
{
	/*if (m_hProvider)
	{
		::CryptReleaseContext(m_hProvider, 0);
	}*/
}
long long  Randfun::GetRealRandom(long long  llMax)
{
	/*if (llMax == 0) return 0;
	static const unsigned int      dwLength = 8;
	static unsigned char             pbBuffer[dwLength] = { 0 };
	bool bIsSucc = ::CryptGenRandom(m_hProvider, dwLength, pbBuffer);
	unsigned long long  llRand = 0;
	if (!bIsSucc)
	{
		return (long long )(rand() / (double)(RAND_MAX + 1) * llMax);
	}
	llRand = *(unsigned long long *)pbBuffer;

	return (long long )(llRand % llMax);*/
	//CryptGenRandom()
	return 0;
}
long long  Randfun::GetRealRandom(long long  llMax, long long  llMin)
{
	return GetRealRandom(llMax - llMin) + llMin;
	return 0;
}
