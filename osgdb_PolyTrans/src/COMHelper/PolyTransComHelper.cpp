#include <osg/Notify>
#include <osgDB/FileNameUtils>
#include "COMHelper/shared_okino_com_src.h"
#include "COMHelper/PolyTransCOMSinkEvents.h"
#include "COMHelper/PolyTransOutputConsole.h"
#include "COMHelper/polytranscomhelper.h"
#include "COMHelper/PolyTransImporter.h"

#include <fstream>
#include <cctype>

PolyTransComHelper::PolyTransComHelper()
:
    m_ConfigValue_BaseIntermediateFileName( "" ),
    m_ConfigValue_IntermediateFileType( "ive" ),
    m_ShowExportOptionsWindow( true ),
    m_ShowImportOptionsWindow( true )
{
    // Initialize the pNuGrafIO and pConvertIO pointers to NULL
    OkinoCommonComSource___Initialization();
}


PolyTransComHelper::~PolyTransComHelper(void)
{
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
    PluginMap::const_iterator it = m_PolyTransImporters.begin();
    while (it != m_PolyTransImporters.end())
    {
        std::string ext( (*it).first );
        if (osgDB::equalCaseInsensitive( ext, fileExtension ) )
        {
            importerNameToUse = (*it).second;
            break;
        }
        it++;
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
    std::string guid;
    if ( m_ConfigValue_IntermediateFileType == "flt" )
        guid = "{7F686138-849F-4a88-B46D-3B5BD611A342}";
    else if ( m_ConfigValue_IntermediateFileType == "obj" )
        guid = "{C0E402F6-6796-4284-A8FC-FCCFBEE6EF84}";
    else
        // Default, either .osg or .ive. Either way use the OSG exporter.
        guid = "{E824F7D3-1307-4dcf-B31A-CCD0A712C066}";

    const std::string fileNameAbsolute = ComputeIntermediateFileNameAndPath();
    bool exportSuccess( true );
    if ( fileNameAbsolute.length() > 0 )
    {
        short exportStatus = OkinoCommonComSource___HandleCompleteFileExportProcess(
            parent_hwnd,
            (char*)( fileNameAbsolute.c_str() ),
            (char*)( guid.c_str() ),
            m_ShowExportOptionsWindow, //show export options dialog
            FALSE ); //create backup file (if destination file already exists in targer folder?)

        if ( exportStatus == 0 )
            exportSuccess = false;
    }
    else
        exportSuccess = false;

    return( exportSuccess );
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


void
PolyTransComHelper::SetShowImportOptionsWindow( bool showWindow )
{
    m_ShowImportOptionsWindow = showWindow;
}

void
PolyTransComHelper::setPluginPreferences( const PluginMap& polyTransPluginPreferences )
{
    m_PolyTransImporters = polyTransPluginPreferences;
}

void
PolyTransComHelper::setIntermediateFileNameExt( const std::string& ext )
{
    m_ConfigValue_IntermediateFileType = ext;
}

void
PolyTransComHelper::setIntermediateFileNameBase( const std::string& base )
{
    m_ConfigValue_BaseIntermediateFileName = base;
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


// Create the intermediate file name and path. We tell PolyTrans to export to this file and path.
// The actual location might not be the same when exporting to flt, as the flt export dialog
// allows the user to export to a subdirectory.
//
// Default behavior is to use the same base file name and path as the input file, with the .ive
// extension, however all this may be overridden in the config file.
std::string
PolyTransComHelper::ComputeIntermediateFileNameAndPath( const std::string& srcFile ) const
{
    std::string intermediateFileNameAndPath = "";
    
    if ( m_ConfigValue_BaseIntermediateFileName.length() == 0 )
    {
        osg::notify( osg::INFO ) << "osgdb_PolyTrans: Default intermediate file and path:" << std::endl;

        std::string inFile;
        if (!m_FileNameAndPathToImport.empty())
            inFile = m_FileNameAndPathToImport;
        else
            inFile = srcFile;

        std::string possibleBaseName( osgDB::getNameLessExtension( inFile ) );
        std::string possibleExtension( osgDB::getFileExtension( possibleBaseName ) );
        if( possibleExtension.empty() )
        {
            // There was no 2nd extension.
            intermediateFileNameAndPath = possibleBaseName;
        }
        else
        {
            // There was a 2nd extension, e.g., filename.asm.1
            intermediateFileNameAndPath = osgDB::getNameLessExtension( possibleBaseName );
        }
    }
    else
    {
        osg::notify( osg::INFO ) << "osgdb_PolyTrans: Specified intermediate file and path:" << std::endl;

        if ( m_ConfigValue_BaseIntermediateFileName.find_first_of( ':' ) != -1 )
        {
            intermediateFileNameAndPath = m_ConfigValue_BaseIntermediateFileName;
            //then we'll assume that this is an absolute path, so keep it like it is
        }
        else
        {
            intermediateFileNameAndPath = m_exportPath;

            if ( ( m_ConfigValue_BaseIntermediateFileName[ 0 ] != '/' ) && ( m_ConfigValue_BaseIntermediateFileName[ 0 ] != '\\' ) )
                intermediateFileNameAndPath += "\\";
            intermediateFileNameAndPath += m_ConfigValue_BaseIntermediateFileName;
        }
    }

    intermediateFileNameAndPath += ".";
    intermediateFileNameAndPath += m_ConfigValue_IntermediateFileType;
    osg::notify( osg::INFO ) << "osgdb_PolyTrans:   " << intermediateFileNameAndPath << std::endl;

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

// Query PolyTrans for all supported extensions. Some PolyTrans plugins support multiple
//   extensions, which we get as a space- or semicolon-separated char*. Parse out
//   each individual extension and store it in a std::list<std::string>, and return it.
PolyTransComHelper::ExtensionList
PolyTransComHelper::getSupportedExtensions()
{
    Nd_Import_Converter_Info_List* listOfImporters = 
            OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();
    int numImporters = listOfImporters[0].num_importers_in_list;

    ExtensionList el;
    for ( int i=0; i<numImporters; i++ )
    {
        Nd_Import_Converter_Info_List* importerInList = &( listOfImporters[ i ] );
        std::string manyExts( importerInList->file_extensions );
        if (manyExts.empty())
            continue;

        std::string::size_type startIdx = manyExts.find_first_of( "." );
        std::string::size_type endIdx = manyExts.find_first_of( " ;" );
        while (startIdx < endIdx)
        {
            const std::string ext = manyExts.substr( startIdx+1, endIdx-(startIdx+1) );
            el.push_back( ext );
            if (endIdx == manyExts.npos)
                break;

            startIdx = endIdx+1;
            endIdx = manyExts.find_first_of( " ;", startIdx );
        }
    }

    return el;
}

