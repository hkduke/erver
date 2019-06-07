#pragma once
#include "acl_cpp/lib_acl.hpp"

class CBuildDBThread : public acl::thread 
{
public:
	CBuildDBThread() {}
	~CBuildDBThread() {}

private:
	void AddRecordScoreStuffTable();
	void AddRecordStuffTable();
	void AddPRoomPlaybackTable();

protected:
	virtual void* run();
};