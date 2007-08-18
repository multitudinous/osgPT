#include "COMHelper/polytransoutputconsole.h"
#include <stdio.h>


PolyTransOutputConsole::PolyTransOutputConsole(void) :
	m_ConsoleWindowHandle( NULL ),
	m_CreatedConsoleWindow( false )
{
	BOOL success = AllocConsole();

	if ( success != 0 )
	{
		SetConsoleTitle( "PolyTrans COM Output" );
		COORD co = { 80, 275 };
		SetConsoleScreenBufferSize( m_ConsoleWindowHandle, co );
		m_CreatedConsoleWindow = true;
	}

	m_ConsoleWindowHandle = GetStdHandle( STD_OUTPUT_HANDLE );
}


PolyTransOutputConsole::~PolyTransOutputConsole(void)
{
	//Maybe it would be good to keep the console window open in case the
	// user wants to look at it after the conversion has finished.
	//if ( m_CreatedConsoleWindow )
	//{
	//	FreeConsole();
	//}
}


void
PolyTransOutputConsole::WriteLineToConsole( char* aLineToWrite )
{
	DWORD numCharsWritten;

	WriteConsole( m_ConsoleWindowHandle,
		aLineToWrite,
		(DWORD)( strlen( aLineToWrite ) ),
		&numCharsWritten,
		NULL );

	char returnChar[3];
	strcpy( returnChar, "\n" );

	WriteConsole( m_ConsoleWindowHandle,
		returnChar,
		(DWORD)( strlen( returnChar ) ),
		&numCharsWritten,
		NULL );
}


void
PolyTransOutputConsole::WriteErrorMessage( char* anErrorLevel, char* anErrorLabel, char* aFormattedMessage )
{
	char errorMessage[ 500 ];
	strcpy( errorMessage, "PolyTrans Error: " );
	strcat( errorMessage, "Level: " );
	strcat( errorMessage, anErrorLevel );
	strcat( errorMessage, " Label: " );
	strcat( errorMessage, anErrorLabel );
	strcat( errorMessage, " Message: " );
	strcat( errorMessage, aFormattedMessage );

	WriteLineToConsole( errorMessage );
}


void
PolyTransOutputConsole::WriteImportProgress( char* aMessageToWrite )
{
	WriteLineToConsole( aMessageToWrite );
}


void
PolyTransOutputConsole::WriteExportMessage( char* anExportMessage )
{
	char exportMessage[300];
	strcpy( exportMessage, "PolyTrans Export: " );

	strcat( exportMessage, anExportMessage );
	WriteLineToConsole( exportMessage );
}


void
PolyTransOutputConsole::WriteExportProgress( long aLineNum, long aTotalNumLines )
{
	char progressMessage[ 500 ];
	strcpy( progressMessage, "PolyTrans Export Progress: Total Lines: " );
	
	char totalLinesChar[ 100 ];
	if ( aTotalNumLines >= 0 )
	{
		sprintf( totalLinesChar, "%ld", aTotalNumLines );
	}
	else
	{
		strcpy( totalLinesChar, "0" );
	}
	strcat( progressMessage, totalLinesChar );

	strcat( progressMessage, " LineNum: " );
	char lineNumChar[ 100 ];
	sprintf( lineNumChar, "%ld", aLineNum );
	strcat( progressMessage, lineNumChar );

	WriteLineToConsole( progressMessage );
}


void
PolyTransOutputConsole::WriteMessage( char* aMessageToWrite )
{
	WriteLineToConsole( aMessageToWrite );
}
