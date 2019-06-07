#ifndef POPCAP_SERVER_HEADER_FILE_MSGID_H
#define POPCAP_SERVER_HEADER_FILE_MSGID_H

enum _MsgSegment {
	MSGSEG_START								= 1000,


};

enum _MsgCommand {
	//Login <-> Clients
	MC_C2S_Grab_Landlord												= MSGSEG_START + 1,
	MC_S2C_Grab_Landlord_Result											= MSGSEG_START + 2,
	MC_C2S_Play_Card													= MSGSEG_START + 3,
	MC_S2C_Play_Card_Result												= MSGSEG_START + 4,
	MC_C2S_Passive_Play_Card											= MSGSEG_START + 5,
	MC_S2C_Passive_Play_Card_Result										= MSGSEG_START + 6,
};

#endif	//#ifndef POPCAP_SERVER_HEADER_FILE_MSGID_H

