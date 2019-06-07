
#ifndef __Random_h__
#define __Random_h__
//#include <wincrypt.h>

class Randfun
{
private:
	Randfun();
	~Randfun();
public:

	// �������������
	static void SetRandomSeed(unsigned int dwSeed);


	// ��ȡ�����
	static long long  GetRandom(long long  llMax, long long  llMin);
	// ��ʼ�������
	static bool InitRealRandom();
	// �ͷ������
	static void ReleaseRealRandom();
	// ��ȡ�����
	static long long  GetRealRandom(long long  llMax);
	// ��ȡ�����
	static long long  GetRealRandom(long long  llMax, long long  llMin);

protected:
	static unsigned int m_dwSeed;
	//static HCRYPTPROV       m_hProvider;
};
#endif // __Random_h__

