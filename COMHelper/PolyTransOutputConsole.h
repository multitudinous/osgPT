#pragma once

#include "stdafx.h"
#include "PolyTransOutput.h"


class PolyTransOutputConsole : public PolyTransOutput
{

public:

	//CONSTRUCTOR
	PolyTransOutputConsole(void);

	//DESTRUCTOR
	~PolyTransOutputConsole(void);

	void WriteErrorMessage( char* anErrorLevel, char* anErrorLabel, char* aFormattedMessage ); //override
	void WriteExportMessage( char* anExportMessage ); //override
	void WriteExportProgress( long aLineNum, long aTotalNumLines ); //override
	void WriteImportProgress( char* aMessageToWrite ); //override
	void WriteLineToConsole( char* aLineToWrite );
	void WriteMessage( char* aMessageToWrite ); //override

private:


	//DATA MEMBERS
	HANDLE m_ConsoleWindowHandle;
	bool m_CreatedConsoleWindow;
};
