#pragma once

class PolyTransOutput
{

public:

	PolyTransOutput(void);

	virtual ~PolyTransOutput(void);

	virtual void WriteErrorMessage( char* anErrorLevel, char* anErrorLabel, char* aFormattedMessage ) = 0;
	virtual void WriteExportMessage( char* anExportMessage ) = 0;
	virtual void WriteExportProgress( long aLineNum, long aTotalNumLines ) = 0;
	virtual void WriteImportProgress( char* aMessageToWrite ) = 0;
	virtual void WriteMessage( char* aMessageToWrite ) = 0;
};
