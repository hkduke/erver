/* ----------------------------------------------------------------------------------------------------------




-----------------------------------------------------------------------------------------------------------*/
#include "../DdzAiLib/Log.h"
#include "../DdzAiLib/DdzAiInterface.h"

#include<sys/types.h>
#include<sys/stat.h>
#include <iostream>

/*---------------------------------------------------------------------------------------------------------*/
ddz_ai_interface*	g_pDdzAi = 0;

int Init( int iMode );
int Daemon( int iMode );
void SignHandle1( int iSignValue );
void SignHandle2( int iSignValue );

void get_result(int event, int handle, const pai_interface_move* move)
{
	Log_Msg(Log_Debug, "event=%d handle=%d-------------->type=%d alone1=%d alone2=%d combo_count=%d", event, handle, move->_type, move->_alone_1, move->_alone_2, move->_combo_count);

}

/*---------------------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
	Log_Init("DdzAiSample");


	g_pDdzAi = new ddz_ai_interface;
	if ( !g_pDdzAi )
	{
		Log_Msg( Log_Failed, "DdzAi: Create DdzAi Failed!\n" );
		return 0;
	}

	if ( g_pDdzAi->Initialize( "./DdzAi.cfg" ) != 0 )
	{
		Log_Msg( Log_Failed, "DdzAi: Initialize() Failed!\n" );
		return 0;
	}

	g_pDdzAi->SetPlayCardInterface(get_result);

	Log_Msg( Log_Normal, "Started.\n" );

	unsigned char pai_list[pai_i_type_max];
	unsigned char pai_list2[pai_i_type_max];
	unsigned char pai_list3[pai_i_type_max];

	memset(pai_list, 0, sizeof(unsigned char)*pai_i_type_max);
	pai_list[pai_i_type_3] = 0;
	pai_list[pai_i_type_4] = 0;
	pai_list[pai_i_type_5] = 1;
	pai_list[pai_i_type_6] = 1;
	pai_list[pai_i_type_7] = 1;
	pai_list[pai_i_type_8] = 1;
	pai_list[pai_i_type_9] = 1;
	pai_list[pai_i_type_10] = 1;
	pai_list[pai_i_type_J] = 1;
	pai_list[pai_i_type_Q] = 1;
	pai_list[pai_i_type_K] = 2;
	pai_list[pai_i_type_A] = 1;
	pai_list[pai_i_type_2] = 0;
	pai_list[pai_i_type_blackjack] = 0; 
	pai_list[pai_i_type_blossom] = 0; 

	memset(pai_list2, 0, sizeof(unsigned char)*pai_i_type_max);
	pai_list2[pai_i_type_3] = 2;
	pai_list2[pai_i_type_4] = 2;
	pai_list2[pai_i_type_5] = 2;
	pai_list2[pai_i_type_6] = 0;
	pai_list2[pai_i_type_7] = 2;
	pai_list2[pai_i_type_8] = 0;
	pai_list2[pai_i_type_9] = 2;
	pai_list2[pai_i_type_10] = 2;
	pai_list2[pai_i_type_J] = 0;
	pai_list2[pai_i_type_Q] = 0;
	pai_list2[pai_i_type_K] = 0;
	pai_list2[pai_i_type_A] = 0;
	pai_list2[pai_i_type_2] = 0;
	pai_list2[pai_i_type_blackjack] = 0; 
	pai_list2[pai_i_type_blossom] = 0; 

	memset(pai_list3, 0, sizeof(unsigned char)*pai_i_type_max);
	pai_list3[pai_i_type_3] = 0;
	pai_list3[pai_i_type_4] = 1;
	pai_list3[pai_i_type_5] = 0;
	pai_list3[pai_i_type_6] = 1;
	pai_list3[pai_i_type_7] = 0;
	pai_list3[pai_i_type_8] = 0;
	pai_list3[pai_i_type_9] = 0;
	pai_list3[pai_i_type_10] = 0;
	pai_list3[pai_i_type_J] = 3;
	pai_list3[pai_i_type_Q] = 0;
	pai_list3[pai_i_type_K] = 0;
	pai_list3[pai_i_type_A] = 0;
	pai_list3[pai_i_type_2] = 2;
	pai_list3[pai_i_type_blackjack] = 0; 
	pai_list3[pai_i_type_blossom] = 0; 

	unsigned int handle = g_pDdzAi->CreateAi();
//	g_pDdzAi->AiPlayCard(20, pai_list, pai_list2, pai_list3, 2);//, 2, &move);

	pai_interface_move move;
	move._type = 6;
	move._combo_count = 7;
	move._combo_list[0] = 4;
	move._combo_list[1] = 5;
	move._combo_list[2] = 6;
	move._combo_list[3] = 7;
	move._combo_list[4] = 8;
	move._combo_list[5] = 9;
	move._combo_list[6] = 10;

	unsigned char grabpai[3] = {pai_i_type_2, pai_i_type_2, pai_i_type_A};

//	g_pDdzAi->AiPlayCardPassive(21, pai_list, pai_list2, pai_list3, 2, 1, &move);
	int sss = 0;
	while(1)
	{
		g_pDdzAi->Update();
		if (sss++ == 0)
//			g_pDdzAi->GrabLandlord2(3, pai_list, pai_list2, pai_list3, grabpai);
			g_pDdzAi->AiPlayCardPassive(21, pai_list, pai_list2, pai_list3, 0, 2, &move);

	}

	if ( g_pDdzAi )
	{
		delete g_pDdzAi;
		g_pDdzAi = 0;
	}

	return 0;
}


/* ----------------------------------------------------------------------------------------------------------
End of file */

