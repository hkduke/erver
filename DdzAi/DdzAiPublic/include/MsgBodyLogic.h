/* ----------------------------------------------------------------------------------------------------------

Popcap Shanghai studio

Created by Geng Yong in 2009-03-27

Added encrypt and decrypt by Wang Ning 2009-11-02

Redefined by Shen Bo in 2010-06-21
------------------------------------------------------------------------------------------------------------*/
#ifndef POPCAP_SERVER_HEADER_FILE_MSGBODYLOGIN_H
#define POPCAP_SERVER_HEADER_FILE_MSGBODYLOGIN_H

#include "MsgDefine.h"
#include "MsgDefineMacro.h"

struct Msg_DdzMove
{
	unsigned char				_type;
	unsigned char				_alone_1;
	unsigned char				_alone_2;
	unsigned char				_alone_3;
	unsigned char				_alone_4;
	unsigned char				_airplane_pairs;

	unsigned char				_combo_list[20];
	unsigned char				_combo_count;
};

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_C2S_Grab_Landlord, C2S_Grab_Landlord )
	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle)
		for(int i = 0; i < 18; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		for(int i = 0; i < 3; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_vthreePaiList[i]);
		}
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle)
		for(int i = 0; i < 18; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		for(int i = 0; i < 3; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_vthreePaiList[i]);
		}
	Declare_EncodeMsg_Function_End

	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList1, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList2, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList3, 18)
	Declare_Msg_Member_Array(unsigned char, m_vthreePaiList, 3)


Declare_Msg_End

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_S2C_Grab_Landlord_Result, S2C_Grab_Landlord_Result )

	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_DecodeMsg_UnsignedInt(m_uiScore)
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_EncodeMsg_UnsignedInt(m_uiScore)
	Declare_EncodeMsg_Function_End

	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member(unsigned int, m_uiScore)
Declare_Msg_End

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_C2S_Play_Card, C2S_Play_Card )
	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle);
		for(int i = 0; i < 18; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		Declare_DecodeMsg_UnsignedChar(m_ucLandlordIndex);
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle);
		for(int i = 0; i < 18; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		Declare_EncodeMsg_UnsignedChar(m_ucLandlordIndex);
	Declare_EncodeMsg_Function_End
	
	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList1, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList2, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList3, 18)
	Declare_Msg_Member(unsigned char, m_ucLandlordIndex)

Declare_Msg_End

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_S2C_Play_Card_Result, S2C_Play_Card_Result )

	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._type)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_1)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_2)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_3)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_4)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._airplane_pairs)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._combo_count)
		if (m_oDdzMove._combo_count > 20)
			m_oDdzMove._combo_count = 20;
		for(int i = 0; i < m_oDdzMove._combo_count; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_oDdzMove._combo_list[i])
		}
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._type)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_1)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_2)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_3)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_4)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._airplane_pairs)
		if (m_oDdzMove._combo_count > 20)
			m_oDdzMove._combo_count = 20;
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._combo_count)
		for(int i = 0; i < m_oDdzMove._combo_count; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_oDdzMove._combo_list[i])
		}
	Declare_EncodeMsg_Function_End

	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member(Msg_DdzMove, m_oDdzMove)

Declare_Msg_End

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_C2S_Passive_Play_Card, C2S_Passive_Play_Card )
	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle);
		for(int i = 0; i < 18; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_DecodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		Declare_DecodeMsg_UnsignedChar(m_ucLandlordIndex);
		Declare_DecodeMsg_UnsignedChar(m_ucOutPaiIndex);

		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._type)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._alone_1)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._alone_2)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._alone_3)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._alone_4)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._airplane_pairs)
		Declare_DecodeMsg_UnsignedChar(m_oLimitPai._combo_count)
		if (m_oLimitPai._combo_count > 20)
			m_oLimitPai._combo_count = 20;
		for(int i = 0; i < m_oLimitPai._combo_count; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_oLimitPai._combo_list[i])
		}
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle);
		for(int i = 0; i < 18; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_vPaiList1[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList2[i]);
			Declare_EncodeMsg_UnsignedChar(m_vPaiList3[i]);
		}
		Declare_EncodeMsg_UnsignedChar(m_ucLandlordIndex);
		Declare_EncodeMsg_UnsignedChar(m_ucOutPaiIndex);

		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._type)
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._alone_1)
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._alone_2)
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._alone_3)
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._alone_4)
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._airplane_pairs)
		if (m_oLimitPai._combo_count > 20)
			m_oLimitPai._combo_count = 20;
		Declare_EncodeMsg_UnsignedChar(m_oLimitPai._combo_count)
		for(int i = 0; i < m_oLimitPai._combo_count; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_oLimitPai._combo_list[i])
		}

	Declare_EncodeMsg_Function_End

	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList1, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList2, 18)
	Declare_Msg_Member_Array(unsigned char, m_vPaiList3, 18)
	Declare_Msg_Member(unsigned char, m_ucLandlordIndex)
	Declare_Msg_Member(unsigned char, m_ucOutPaiIndex)
	Declare_Msg_Member(Msg_DdzMove, m_oLimitPai)

Declare_Msg_End

/*---------------------------------------------------------------------------------------------------------*/
Declare_Msg_Begin( MC_S2C_Passive_Play_Card_Result, S2C_Passive_Play_Card_Result )
	Declare_DecodeMsg_Function_Begin
		Declare_DecodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._type)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_1)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_2)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_3)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._alone_4)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._airplane_pairs)
		Declare_DecodeMsg_UnsignedChar(m_oDdzMove._combo_count)
		if (m_oDdzMove._combo_count > 20)
			m_oDdzMove._combo_count = 20;
		for(int i = 0; i < m_oDdzMove._combo_count; ++i)
		{
			Declare_DecodeMsg_UnsignedChar(m_oDdzMove._combo_list[i])
		}
	Declare_DecodeMsg_Function_End

	Declare_EncodeMsg_Function_Begin
		Declare_EncodeMsg_UnsignedInt(m_uiAiHandle)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._type)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_1)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_2)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_3)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._alone_4)
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._airplane_pairs)
		if (m_oDdzMove._combo_count > 20)
			m_oDdzMove._combo_count = 20;
		Declare_EncodeMsg_UnsignedChar(m_oDdzMove._combo_count)
		for(int i = 0; i < m_oDdzMove._combo_count; ++i)
		{
			Declare_EncodeMsg_UnsignedChar(m_oDdzMove._combo_list[i])
		}
	Declare_EncodeMsg_Function_End

	Declare_Msg_Member(unsigned int, m_uiAiHandle)
	Declare_Msg_Member(Msg_DdzMove, m_oDdzMove)

Declare_Msg_End

#endif	
