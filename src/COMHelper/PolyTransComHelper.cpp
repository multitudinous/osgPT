#include "osgOQ/ConfigFileReader.h"
#include "COMHelper/shared_okino_com_src.h"
#include "COMHelper/PolyTransCOMSinkEvents.h"
#include "COMHelper/PolyTransOutputConsole.h"
#include "COMHelper/polytranscomhelper.h"
#include "COMHelper/PolyTransImporter.h"

#include <fstream>
#include <cctype>

PolyTransComHelper::PolyTransComHelper() :
	m_ConfigProperty_AppWindowName( "COMHelper_AppWindowName" ), //name that appears on titlebar of parent
	m_ConfigProperty_BaseIntermediateFileName( "COMHelper_BaseIntermediateFileName" ), //relative or absolute path, and filename
	m_ConfigProperty_IntermediateFileType( "COMHelper_IntermediateFileType" ),
	m_ConfigProperty_ShowExportOptions( "COMHelper_ShowExportOptions" ), // Y or N
	m_ConfigProperty_ShowImportOptions( "COMHelper_ShowImportOptions" ), // Y or N
	m_ConfigProperty_PolyTransINIFileName( "COMHelper_PolyTransINIFileName" ), //absolute path and filename
	m_ConfigProperty_PolyTransImporter( "PolyTransImporter" ),
	m_ConfigValue_BaseIntermediateFileName( "" ),
	m_ConfigValue_PolyTransIniFileNameAbsolute( "" ),
	m_ConfigValue_IntermediateFileType( "" ),
	m_ShowExportOptionsWindow( true ),
	m_ShowImportOptionsWindow( true )
{
	// Initialize the pNuGrafIO and pConvertIO pointers to NULL
	OkinoCommonComSource___Initialization();
}


PolyTransComHelper::~PolyTransComHelper(void)
{
	while ( m_ListOfPolyTransImporters.size() > 0 )
	{
		PolyTransImporter* lastImporter = m_ListOfPolyTransImporters.back();
		delete lastImporter;

		m_ListOfPolyTransImporters.pop_back();
	}
}


bool
PolyTransComHelper::AttachToPolyTransCom( HINSTANCE anAppInstance )
{
	bool attachSuccess = true;

	if ( m_AppWindowName.size() > 0 )
	{
		parent_hwnd = ::FindWindow( NULL, (LPCTSTR)( m_AppWindowName.c_str() ) );
	}
	if ( parent_hwnd == NULL )
	{
		parent_hwnd = ::GetDesktopWindow();
	}

	if ( parent_hwnd == NULL )
	{
		attachSuccess = false;
	}

	if ( attachSuccess )
	{
		attachSuccess = RegisterPolyTransComServer();
	}

	if ( attachSuccess )
	{
		HINSTANCE dllHInstance = (HINSTANCE)&__ImageBase;

		// Go and attempt to attach to the COM server then to instantiate the
		// pNuGrafIO and pConvertIO class interface pointers. Displays an 
		// error message via Messagebox() if the NuGrafIO or ConvertIO interfaces
		// could not be obtained, or if the event sinks could not be registered.
		BOOL okinoSuccess = ( OkinoCommonComSource___AttachToCOMInterface( parent_hwnd, dllHInstance ) );
		if ( okinoSuccess == FALSE )
		{
			attachSuccess = false;
		}

		if ( attachSuccess )
		{
			bool isLicensed = OkinoCommonComSource___IsCOMServerLicensed();

			if ( !isLicensed )
			{
				//TODO: Finish this part to let the user register PolyTrans.  If they choose not to register,
				// then we probably should abort and not do the conversion.  But, we need to be able to test this.
				OkinoCommonComSource___ShowCOMServerRegistrationDialogBox(
					parent_hwnd,
					FALSE ); //let user do registration online

				attachSuccess = false;
			}
		}
	}

	return attachSuccess;
}


bool
PolyTransComHelper::ResetPolyTrans()
{
	bool resetSuccess = true;

	HRESULT resetResult = pNuGrafIO->Reset( 0 );
	if ( FAILED( resetResult ) )
	{
		resetSuccess = false;
	}

	return resetSuccess;
}	


bool
PolyTransComHelper::ImportModelIntoPolyTrans( const std::string& aFileNameAndPathToImport )
{
	m_FileNameAndPathToImport = aFileNameAndPathToImport;

	bool importSuccess = true;

	std::string fileExtension = GetLowerCaseFullFileExtension( aFileNameAndPathToImport );

    std::string importerNameToUse = "";

	//see if a mapping exists that was loaded from the config file
	int numConfigImporters = static_cast< int >( m_ListOfPolyTransImporters.size() );
	for ( int i = 0; i < numConfigImporters; i++ )
	{
		PolyTransImporter* importerInList = m_ListOfPolyTransImporters[ i ];
		if ( importerInList->GetFileExtension() == fileExtension )
		{
			importerNameToUse = importerInList->GetImporterName();
		}
	}

	std::string importerGUID = "";
	if ( importerNameToUse.size() > 0 )
	{
		importerGUID = GetGUIDForImporterName( importerNameToUse );
	}

	if ( m_FileNameAndPathToImport.length() > 0 )
	{
		char* importerGuidChar = NULL;
		
		if( importerGUID.size() > 0 )
		{
			importerGuidChar = new char[ importerGUID.size() + 1 ];
			strcpy( importerGuidChar, importerGUID.c_str() );
		}

		short status = OkinoCommonComSource___HandleCompleteFileImportProcess(
			parent_hwnd,
			(char*)( m_FileNameAndPathToImport.c_str() ),
			importerGuidChar,
			m_ShowImportOptionsWindow, //show options dialog box
			TRUE,
			TRUE,
			TRUE,
			TRUE ); //ask user for file format if unknown

		if ( importerGuidChar != NULL )
		{
			delete[] importerGuidChar;
			importerGuidChar = NULL;
		}

		char message[ 300 ];

		if ( status == 1 )
		{
			strcpy( message, "OSGDev: Import Completed" );
		}
		else
		{
			strcpy( message, "OSGDev: Import Failed" );
			importSuccess = false;
		}
	}

	return importSuccess;
}


void
PolyTransComHelper::SetExportPath( std::string& exportPath )
{
	m_exportPath = exportPath;
}


bool
PolyTransComHelper::ExportPolyTransModelToIntermediateFile()
{
	bool exportResult = false;
	if ( ( m_ConfigValue_IntermediateFileType == "flt" ) || ( m_ConfigValue_IntermediateFileType == "" ) )
	{
		exportResult = ExportPolyTransModelToOpenFlight();
	}
	else if ( m_ConfigValue_IntermediateFileType == "obj" )
	{
		exportResult = ExportPolyTransModelToWaveFront();
	}

	return exportResult;
}


bool
PolyTransComHelper::ExportPolyTransModelToOpenFlight()
{
	bool exportSuccess = true;

	std::string openFlightFileNameAbsolute = ComputeIntermediateFileNameAndPath();

	if ( openFlightFileNameAbsolute.length() > 0 )
	{
		std::string openflightExporterGuid = "{7F686138-849F-4a88-B46D-3B5BD611A342}";

		short exportStatus = OkinoCommonComSource___HandleCompleteFileExportProcess(
			parent_hwnd,
			(char*)( openFlightFileNameAbsolute.c_str() ),
			(char*)( openflightExporterGuid.c_str() ),
			m_ShowExportOptionsWindow, //show export options dialog
			FALSE ); //create backup file (if .flt file already exists in targer folder?)

		if ( exportStatus == 0 )
		{
			exportSuccess = false;
		}
	}
	else
	{
		exportSuccess = false;
	}

	return exportSuccess;
}


bool
PolyTransComHelper::ExportPolyTransModelToWaveFront()
{
	bool exportSuccess = true;

	std::string wavefrontNameAbsolute = ComputeIntermediateFileNameAndPath();

	if ( wavefrontNameAbsolute.length() > 0 )
	{
		std::string waveFrontExporterGuid = "{C0E402F6-6796-4284-A8FC-FCCFBEE6EF84}";

		short exportStatus = OkinoCommonComSource___HandleCompleteFileExportProcess(
			parent_hwnd,
			(char*)( wavefrontNameAbsolute.c_str() ),
			(char*)( waveFrontExporterGuid.c_str() ),
			m_ShowExportOptionsWindow, //show export options dialog
			FALSE ); //create backup file (if .flt file already exists in targer folder?)

		if ( exportStatus == 0 )
		{
			exportSuccess = false;
		}
	}
	else
	{
		exportSuccess = false;
	}

	return exportSuccess;
}


void
PolyTransComHelper::DetachFromPolyTransCOM()
{
	// Release pointers to the interfaces. This will also cause the server
	// (nugraf32.exe or pt32.exe) to exit if the pNuGrafIO->ExitServerProgramWhenLastCOMClientDetaches()
	// function has been called with a TRUE argument. 
	OkinoCommonComSource___DetachFromCOMInterface();
}


std::string
PolyTransComHelper::GetGUIDForImporterName( const std::string& anImporterName )
{
	std::string importerGUID = "";

	Nd_Import_Converter_Info_List* listOfImporters = 
			OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();
	int numImporters = listOfImporters[0].num_importers_in_list;

	// First one is a special case, Okino .bdf format
	for ( int i = 0; ( importerGUID == "" ) && ( i < numImporters ); i++ )
	{
		Nd_Import_Converter_Info_List* importerInList = &( listOfImporters[ i ] );
		if ( importerInList->plugin_descriptive_name == anImporterName )
		{
			importerGUID = importerInList->guid;
		}
	}

	return importerGUID;
}


//This is helpful for looping through the list of Okino importers to get information about each importer,
// such as its GUID or list of supported file extensions.
void
PolyTransComHelper::SearchListOfImporters()
{
	Nd_Import_Converter_Info_List* curr_importer_list = 
			OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();
		
	// First one is a special case, Okino .bdf format
	Nd_Import_Converter_Info_List* importerInList = NULL;
	for ( int i = 0; i < curr_importer_list[0].num_importers_in_list; i++ )
	{
		importerInList = &( curr_importer_list[ i ] );
	}
}


void
PolyTransComHelper::SearchListOfExporters()
{
	Nd_Export_Converter_Info_List* listOfExporters = OkinoCommonComSource___GetListofExportersAndTheirAttributesFromCOMServer();

	Nd_Export_Converter_Info_List* exporterInList = NULL;
	for ( int i = 0; i < listOfExporters[0].num_exporters_in_list; i++ )
	{
		exporterInList = &( listOfExporters[ i ] );
	}
}


//This function will return a file extension beginning with the first dot that is found, and
// extending to the end of the string. So, if a filename contains more than one dot, for exampe:
// modelFileName.asm.1, then the value returned would be: asm.1
std::string
PolyTransComHelper::GetLowerCaseFullFileExtension( const std::string& aFileName )
{
	int firstDotIndex = (int)( aFileName.find_first_of( '.' ) );
    std::string tempExt( aFileName.begin()+firstDotIndex+1, aFileName.end() );
    //See if the extension is only 1 char which would mean a ProE file
    if( tempExt.size() == 1 )
    {
        //double check and make sure the extension is a digit
        if( std::isdigit( tempExt.at( 0 ) ) )
        {
            //if so then grab the real index before the 3 space extension
            firstDotIndex = 
                (int)( aFileName.find_last_of( '.', firstDotIndex - 1 ) );
        }
    }

	std::string lowerCaseExtension = "";
	for ( int i = firstDotIndex + 1; i < (int)( aFileName.size() ); i++ )
	{
		char lowerChar = tolower( aFileName[ i ] );
		lowerCaseExtension += lowerChar;
	}
	return lowerCaseExtension;
}


//By calling this function, PolyTransComHelper will extract from the config file properties
// that are understood by this object.  If any are found, then the default values will be
// replaced by the values in the file.  The parameter aConfigFileName must be an absolute
// path and filename.
void
PolyTransComHelper::LoadOptionsFromConfigFile( ConfigFileReader& configFileReader )
{
	if (!configFileReader.valid())
		return;

	if ( configFileReader.HasProperty( m_ConfigProperty_AppWindowName ) )
	{
		m_AppWindowName = configFileReader.GetValue( m_ConfigProperty_AppWindowName );
	}

	if ( configFileReader.HasProperty( m_ConfigProperty_ShowImportOptions ) )
	{
		std::string showImportOptionsCode = configFileReader.GetValue( m_ConfigProperty_ShowImportOptions );
		if ( showImportOptionsCode == "Y" )
		{
			m_ShowImportOptionsWindow = true;
		}
		else
		{
			m_ShowImportOptionsWindow = false;
		}
	}

	if ( configFileReader.HasProperty( m_ConfigProperty_ShowExportOptions ) )
	{
		std::string showExportOptionsCode = configFileReader.GetValue( m_ConfigProperty_ShowExportOptions );
		if ( showExportOptionsCode == "Y" )
		{
			m_ShowExportOptionsWindow = true;
		}
		else
		{
			m_ShowExportOptionsWindow = false;
		}
	}

	if ( configFileReader.HasProperty( m_ConfigProperty_BaseIntermediateFileName ) )
	{
		m_ConfigValue_BaseIntermediateFileName = configFileReader.GetValue( m_ConfigProperty_BaseIntermediateFileName );
	}

	if ( configFileReader.HasProperty( m_ConfigProperty_PolyTransINIFileName ) )
	{
		m_ConfigValue_PolyTransIniFileNameAbsolute = configFileReader.GetValue( m_ConfigProperty_PolyTransINIFileName );
	}

	if ( configFileReader.HasProperty( m_ConfigProperty_IntermediateFileType ) )
	{
		m_ConfigValue_IntermediateFileType = configFileReader.GetValue( m_ConfigProperty_IntermediateFileType );
	}
	
	if ( configFileReader.HasProperty( m_ConfigProperty_PolyTransImporter ) )
	{
		std::vector< std::string > listOfConfigValues;
		configFileReader.GetValues( m_ConfigProperty_PolyTransImporter, listOfConfigValues );

		int numImporters = static_cast< int >( listOfConfigValues.size() );
		for ( int i = 0; i < numImporters; i++ )
		{
			std::string configValueInList = listOfConfigValues[ i ];

			int firstBlank = static_cast< int >( configValueInList.find_first_of( ' ' ) );
			std::string fileExtension = configValueInList.substr( 0, firstBlank );
			std::string importerName = configValueInList.substr( firstBlank + 1 );

			PolyTransImporter* newPolyTransImporter = new PolyTransImporter( fileExtension, importerName );
			m_ListOfPolyTransImporters.push_back( newPolyTransImporter );
		}
	}
}


void
PolyTransComHelper::SetShowImportOptionsWindow( bool showWindow )
{
	m_ShowImportOptionsWindow = showWindow;
}


void
PolyTransComHelper::SetShowExportOptionsWindow( bool showWindow )
{
	m_ShowExportOptionsWindow = showWindow;
}


bool
PolyTransComHelper::RegisterPolyTransComServer()
{
	bool registerSuccess = true;

	// Okino recommends registering the com server dll each
	// time you run the application. If we don't call this, then the user is required to run PolyTrans.exe at
	// least one time after installing it to register the .dll.  If they do that, then calling this code is
	// not required.  There are two issues I found
	// 1) While testing, I got an error message saying that it could not find vc4_nu.dll.  Not sure why, but
	//    that file is located in c:\\Program Files\\polytrans\\vc4_nu.dll.  Could this require a change in
	//    the system Path variable?
	// 2) Note that it needs the absolute path to ui_dcom.dll.  We need to find the correct path on the user's
	//    computer.

	//CoInitialize(NULL);

	//std::string comServerPathAndFile = "c:\\Program Files\\polytrans\\vcplugin\\ui_dcom.dll";

	//HINSTANCE hInstance = LoadLibrary( (LPCSTR)( comServerPathAndFile.c_str() ) );
	//if ( hInstance < (HINSTANCE)HINSTANCE_ERROR )
	//{
	//	registerSuccess = false;
	//}
	//else
	//{
	//	FARPROC lpDllEntryPoint = (FARPROC)GetProcAddress( hInstance, _T("DllRegisterServer" ) );

	//	if ( lpDllEntryPoint != NULL )
	//	{
	//		(*lpDllEntryPoint)();
	//	}
	//	else
	//	{
	//		registerSuccess = false;
	//	}
	//}

	return registerSuccess;
}


void
PolyTransComHelper::SetAppWindowName( const std::string& anAppWindowName )
{
	m_AppWindowName = anAppWindowName;
}


//this computes the .flt file name that we should set when telling PolyTrans to Export to OpenFlight.  This may
// not be the actual location of the .flt file name that is generated, though, because if the user had put a
// check in the OpenFlight Export Options dialog box telling PolyTrans to create a subfolder, then PolyTrans
// will take the .flt file name and use that name to create a new folder, then put the .flt file in that 
// subfolder.  The reason to call this is because the user is given the option for setting the .flt filename
// in the config file.  If the file is not set in the config file, then we'll compute the .flt filename using
// the name of the file that is being converted to OpenFlight.
std::string
PolyTransComHelper::ComputeIntermediateFileNameAndPath( const std::string& srcFile ) const
{
	std::string intermediateFileNameAndPath = "";
	
	if ( m_ConfigValue_BaseIntermediateFileName.length() == 0 )
	{
		std::string inFile;
		if (!m_FileNameAndPathToImport.empty())
			inFile = m_FileNameAndPathToImport;
		else
			inFile = srcFile;

		//then no filename was given by the user, so we'll 
        //use the same filename as the file that was imported
		int indexOfFirstDot = (int)( inFile.find_last_of( '.' ) );
        std::string tempExt( inFile.begin()+indexOfFirstDot+1, inFile.end() );
        //See if the extension is only 1 char which would mean a ProE file
        if( tempExt.size() == 1 )
        {
            //double check and make sure the extension is a digit
            if( std::isdigit( tempExt.at( 0 ) ) )
            {
                //if so then grab the real index before the 3 space extension
                indexOfFirstDot = 
                    (int)( inFile.find_last_of( '.', indexOfFirstDot - 1 ) );
            }
        }
        
		if ( indexOfFirstDot > 0 )
		{
			intermediateFileNameAndPath = inFile.substr( 0, indexOfFirstDot );
		}
	}
	else
	{
		if ( m_ConfigValue_BaseIntermediateFileName.find_first_of( ':' ) != -1 )
		{
			intermediateFileNameAndPath = m_ConfigValue_BaseIntermediateFileName;
			//then we'll assume that this is an absolute path, so keep it like it is
			//m_ConfigValue_BaseIntermediateFileName = fltFileName;
		}
		else
		{
			intermediateFileNameAndPath = m_exportPath;

			if ( ( m_ConfigValue_BaseIntermediateFileName[ 0 ] == '/' ) || ( m_ConfigValue_BaseIntermediateFileName[ 0 ] == '\\' ) )
			{
				intermediateFileNameAndPath += m_ConfigValue_BaseIntermediateFileName;
			}
			else
			{
				intermediateFileNameAndPath += "\\";
				intermediateFileNameAndPath += m_ConfigValue_BaseIntermediateFileName;
			}
		}
	}

	if ( m_ConfigValue_IntermediateFileType.size() > 0 )
	{
		intermediateFileNameAndPath += ".";
		intermediateFileNameAndPath += m_ConfigValue_IntermediateFileType;
	}
	else
	{
		intermediateFileNameAndPath += ".flt";
	}

	return intermediateFileNameAndPath;
}


//The actual .flt file that is generated depends on the checkbox value in the OpenFlight export options window
// that may be presented to the user during the conversion.  If the export options window is not displayed, then
// PolyTrans will use the checkbox value that was set the last time that it was set, even if it was in a previous
// run.  So, the only way we know for sure if a folder was created to hold the .flt file is to look up the value
// of the checkbox in PolyTrans's .ini file.
std::string
PolyTransComHelper::ComputeGeneratedFileNameAndPath()
{
	std::string generatedFltFileNameAndPath = ComputeIntermediateFileNameAndPath();

	bool isOKToContinue = true;

	std::string polyTransIniFileNameAndPath = "";

	BSTR bstrIniFileName; //BSTR is OLECHAR*, OLECHAR is WCHAR
	
	HRESULT hresult = pNuGrafIO->get_ProgramINIFilename( &bstrIniFileName );
	isOKToContinue = SUCCEEDED( hresult );

	if ( isOKToContinue )
	{
		wchar_t* wcharIniFileName = (wchar_t*)( bstrIniFileName );

		int stringLength = (int)( wcslen( wcharIniFileName ) );

		if ( stringLength > 0 )
		{
			char* polyTransIniFileName = new char[ stringLength + 1];
			size_t sizet = wcstombs( polyTransIniFileName, wcharIniFileName, stringLength );
			polyTransIniFileName[ stringLength ] = 0;

			//Note: in the current version of PolyTrans, the .ini file that contains the value that we need
			// is located in the c:\Windows folder.  If you are using PolyTrans, the name of the file will
			// be POLYTRANS.INI.
			char windowsDirectory[ 501 ];
			::GetWindowsDirectory( windowsDirectory, 500 );
			polyTransIniFileNameAndPath = windowsDirectory;
			polyTransIniFileNameAndPath += "\\";
			polyTransIniFileNameAndPath += polyTransIniFileName;

			delete[] polyTransIniFileName;
		}
		else
		{
			isOKToContinue = false;
		}
	}
	
	if ( isOKToContinue )
	{	
		int newFolderValue = FindPolyTransIniCreateNewFolderValue( polyTransIniFileNameAndPath );

		if ( newFolderValue == 1 )
		{
			//then PolyTrans will create a new folder for the .flt file using the name of the flt file
			// for the new folder
			std::string fltFileNameAbsolute = ComputeIntermediateFileNameAndPath();
			char baseFolder[ 501 ];
			OkinoCommonComSource___ExtractBaseFilePath( (char*)( fltFileNameAbsolute.c_str() ), baseFolder );
			generatedFltFileNameAndPath = baseFolder;

			char newFolderName[ 100 ];
			OkinoCommonComSource___ExtractBaseFileName( (char*)( fltFileNameAbsolute.c_str() ), newFolderName, 1 );
			generatedFltFileNameAndPath += newFolderName;
			generatedFltFileNameAndPath += "\\";

			char fltFileName[ 100 ];
			OkinoCommonComSource___ExtractBaseFileName( (char*)( fltFileNameAbsolute.c_str() ), fltFileName, 0 );
			generatedFltFileNameAndPath += fltFileName;
		}
	}

	return generatedFltFileNameAndPath;
}


//Note that this will not work in the future if Okino decides to change the way they save this value in
// their .ini file!
int
PolyTransComHelper::FindPolyTransIniCreateNewFolderValue( const std::string& aPolyTransIniFileAndPath )
{
	int newFolderValue = -1;

	std::ifstream inputStream;
	inputStream.open( aPolyTransIniFileAndPath.c_str() );

	if ( inputStream.is_open() == true )
	{
		bool continueReadingFile = true;

		char stringToFind0[23];
		strcpy( stringToFind0, "Create-New-Directory=0" );
		char stringToFind1[23];
		strcpy( stringToFind1, "Create-New-Directory=1" );

		char lineOfData[ 501 ];

		while ( continueReadingFile )
		{
			inputStream.getline( lineOfData, 500 );
		
			if ( strlen( lineOfData ) <= 0 )
			{
				continueReadingFile = false;
			}
			
			if ( continueReadingFile )
			{
				if ( strcmp( lineOfData, stringToFind0 ) == 0 )
				{
					newFolderValue = 0;
					continueReadingFile = false;
				}
				else if ( strcmp( lineOfData, stringToFind1 ) == 0 )
				{
					newFolderValue = 1;
					continueReadingFile = false;
				}
			}
		}
	}

	return newFolderValue;
}

//Each PolyTrans Importer has a string property that contains a list of file extensions supported by
// that importer, separated by a semicolon.  This functions checks to see if aFileExtNoDot can be found
// in any one of those lists.
bool
PolyTransComHelper::IsExtensionSupportedByImporters( const std::string& aFileExtNoDot )
{
	bool isSupported = false;

	Nd_Import_Converter_Info_List* listOfImporters = 
			OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();
	int numImporters = listOfImporters[0].num_importers_in_list;

	for ( int i = 0; ( isSupported == false ) && ( i < numImporters ); i++ )
	{
		Nd_Import_Converter_Info_List* importerInList = &( listOfImporters[ i ] );
		
		isSupported = IsExtensionInList( aFileExtNoDot, importerInList->file_extensions );
	}

	return isSupported;
}


//Each PolyTrans Importer has a string property that contains a list of file extensions supported by
// that importer, separated by a semicolon.  This functions checks to see if aFileExtNoDot can be found
// in the list passed in as aListOfExtensions.  (ie. ".igs;.prt" ).
bool
PolyTransComHelper::IsExtensionInList( const std::string& aFileExtNoDot, const std::string& aListOfExtensions )
{
	bool isInList = false;

	std::string extensionInList = "";
	int stringLength = static_cast< int >( aListOfExtensions.size() );
	for ( int i = 0; ( isInList == false ) && ( i < stringLength ); i++ )
	{
		char currentChar = aListOfExtensions[ i ];

		if( ( currentChar == ';' ) || ( currentChar == ' ' ) )
		{
			if ( extensionInList.size() > 0 )
			{
				if ( extensionInList == aFileExtNoDot )
				{
					isInList = true;
				}
			}
			extensionInList = "";
		}
		else
		{
			if ( currentChar != '.' )
			{
				extensionInList +=  currentChar;
			}
			if ( i == ( aListOfExtensions.size() - 1 ) )
			{
				//then we're reading last char in string, so go ahead and check for match:
				if ( extensionInList == aFileExtNoDot )
				{
					isInList = true;
				}
			}
		}
	}
	return isInList;
}
