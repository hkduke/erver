/* ----------------------------------------------------------------------------------------------------------

in in 2010-10-11
----------------------------------------------------------------------------------------------------------*/
#ifndef __ICOMMAND_H__
#define __ICOMMAND_H__

class NetMsg;

class ICommunicationCommand
{
public:
	virtual int Execute( SwitchMsgHead * header, void* pMsg, short nMsgLength, int iConnection ) = 0;
	virtual void Notify() = 0;
};



#endif

