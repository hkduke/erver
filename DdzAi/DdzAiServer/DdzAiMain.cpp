/* ----------------------------------------------------------------------------------------------------------



in 2009-3-19
-----------------------------------------------------------------------------------------------------------*/
#include "DdzAiServer.h"
#include "PopWorldInclude.h"
#include "Log.h"

#include<sys/types.h>
#include<sys/stat.h>

/*---------------------------------------------------------------------------------------------------------*/
DdzAiServer*	g_pDdzAiServer = 0;

int Init( int iMode );
int Daemon( int iMode );
void SignHandle1( int iSignValue );
void SignHandle2( int iSignValue );

/*---------------------------------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
	Log_Init("DdzAiServer");

	int iDeamon = 0;

	if( 1 < argc && !strcasecmp(argv[1], "-ver" ) )
	{
		char szTemp1[100] = {0};
		char szTemp2[100] = {0};
		char szTemp3[100] = {0};

#ifdef _LDEBUG_
		snprintf( szTemp1, sizeof(szTemp1)-1, "debug");
#else
		snprintf( szTemp1, sizeof(szTemp1)-1, "release" );
#endif

#ifdef _POSIX_MT_
		snprintf( szTemp2, sizeof(szTemp2)-1, "multithread");
#else
		snprintf( szTemp2, sizeof(szTemp2)-1, "singlethread" );
#endif


		printf( "DdzAiServer %s %s %s " SERVER_VERSION " build in %s %s\n", szTemp1, szTemp2, szTemp3, __DATE__, __TIME__ );
		return 0;
	}

	int lock_fd = open( "./DdzAiServer.Lock", O_RDWR|O_CREAT, 0640 );
	if ( lock_fd < 0 )
	{
		printf( "DdzAiServer: Open DBCache Server Lock File Failed, DdzAiServer Init Failed!\n" );
		return -1;
	}

	int ret = flock( lock_fd, LOCK_EX | LOCK_NB );
	if( ret < 0 )
	{
		printf("DdzAiServer: Lock File Failed, DdzAiServer is already Running!\n");
		return -1;
	}

	if( 1 < argc && !strcasecmp( argv[1], "-d" ) )
	{
		iDeamon = 1;
	}
	else
	{
		iDeamon = 0;
	}

	int iRet = Init( iDeamon );
	if ( 0 != iRet )
	{
		return 0;
	}

	g_pDdzAiServer = new DdzAiServer;
	if ( !g_pDdzAiServer )
	{
		Log_Msg( Log_Failed, "DdzAiServer: Create DdzAiServer Failed!\n" );
		return 0;
	}

	if ( g_pDdzAiServer->Initialize( "./DdzAiServer.cfg" ) != 0 )
	{
		Log_Msg( Log_Failed, "DdzAiServer: Initialize() Failed!\n" );
		return 0;
	}

	Log_Msg( Log_Normal, "Started.\n" );

	g_pDdzAiServer->Run();

	if ( g_pDdzAiServer )
	{
		delete g_pDdzAiServer;
		g_pDdzAiServer = 0;
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int Init( int iMode )
{
	int nRet = Daemon( iMode );
	if ( nRet != 0 )
	{
		return -1;
	}

	signal( SIGUSR1, SignHandle1 );
	signal( SIGUSR2, SignHandle2 );

	return nRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int Daemon( int iMode )
{
	/* don't change to deamon */
	if ( iMode != 1 )
	{
		return 0;
	}
	else
	{
		Log_Msg( Log_Normal, "DBCacheServer: Daemon...\n" );
	}

	pid_t pid;
	if ( ( pid = fork() ) != 0 )
	{
		/* parent process exit */
		exit( 0 );
	}

	setsid();

	signal( SIGINT,  SIG_IGN );
	signal( SIGHUP,  SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGPIPE, SIG_IGN );
	signal( SIGTTOU, SIG_IGN );
	signal( SIGTTIN, SIG_IGN );
	signal( SIGCHLD, SIG_IGN );
	signal( SIGTERM, SIG_IGN );
	signal( SIGHUP,  SIG_IGN );

	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;

	sigemptyset( &sig.sa_mask );
	sigaction( SIGPIPE, &sig, NULL );

	if ( ( pid = fork() ) != 0 )
	{
		/* parent process exit */
		exit( 0 );
	}

	umask( 0 );
	setpgrp();

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void SignHandle1( int SigValue )
{
	g_pDdzAiServer->SetRunFlag( Dbcsrf_ReConfig );
	signal( SIGUSR1, SignHandle1 );
}

/*---------------------------------------------------------------------------------------------------------*/
void SignHandle2( int SigValue )
{
	g_pDdzAiServer->SetRunFlag( Dbcsrf_DBCacheServerExit );
	signal( SIGUSR2, SignHandle2 );
}

/* ----------------------------------------------------------------------------------------------------------
End of file */

