/*NOTES:

CONFIG FILE PROPERTIES:

Property: COMHelper_AppWindowName
Required: no
Purpose: During the conversion process, PolyTrans will display dialog windows for import options, export options,
         and progress bars.  If you set the name found in the title bar of your main application window, then we
         can use FindWindow() to get the HWND and pass it along to PolyTrans.  If you do not set this value, then
		 we'll use GetDesktopWindow() and pass that HWND to PolyTrans.
Example: COMHelper_AppWindowName TestApplication

Property: COMHelper_BaseIntermediateFileName
Required: no
Purpose: Set the filename, without extension, that will be used for the intermediate file.  If a relative filename
         and path is provided, the file will be placed in the current directory.  If this value is not set in the
		 config file, then the name and location of the file being imported will be used. 
		 Note that if exporting to OpenFlight, a subfolder will be inserted if you check the option in the Export
		 options dialog window to create a subfolder.
Examples:
         COMHelper_BaseIntermediateFileName myfilename 
         COMHelper_BaseIntermediateFileName myfolder/myfilename
         COMHelper_BaseIntermediateFileName /myfolder/myfilename 
         COMHelper_BaseIntermediateFileName \\myfolder\\myfilename

Property: COMHelper_IntermediateFileType
Required: no
Purpose: Used to control the exporter that will be used for the intermediate file that is generated.  Use flt for
         OpenFlight and obj for WaveFront.
Examples:
         COMHelper_IntermediateFileType flt
         COMHelper_IntermediateFileType obj

Property: COMHelper_ShowExportOptions:
Required: no
Purpose: During the convertion, PolyTrans can display a window with the OpenFlight exporter options.  The default
         behavior is to show the window.  Adding this line with an 'N' will prevent the window from being shown.
Examples:
         COMHelper_ShowExportOptions: N  //will prevent export options window from being shown
         COMHelper_ShowExportOptions: Y  //same as if this line was just deleted

Property: COMHelper_ShowImportOptions:
Required: no
Purpose: During the convertion, PolyTrans can display a window with the import options appropriate for the 
         importer that will be used to load the model.  The default behavior is to show the window.  Adding this
		 line with an 'N' will prevent the window from being shown.
Examples:
         COMHelper_ShowImportOptions N  //will prevent import options window from being shown
		 COMHelper_ShowImportOptions Y  //same as if this line was just deleted

Property: PolyTransImporter:
Required: no
Purpose: Used to indicate to PolyTrans which Importer should be used for the given file extension.  This is 
         necessary when more than one importer can support the same file extension.  
Example: PolyTransImporter igs IGES v5.3
         PolyTransImporter igs IGES Files

Property: COMHelper_PolyTransINIFileName:
Required: no
Purpose: The .flt file that is generated could be put in a new folder, depending on that value set in the 
         OpenFlight Export options window.  PolyTrans saves this value in its .ini file.  In the current version
		 of PolyTrans, this file is located in the Windows folder (ie. c:\Windows\\polytrans.ini). Our code
		 uses GetWindowsDirectory() to find this folder and look for a file named polytrans.ini.  If this is not
		 the correct location for this file, then use this config file property to set the absolute path and filename.
Example COMHelper_PolyTransINIFileName: c:\Windows\polytrans.ini

 */

#pragma once

#include "stdafx.h"
#include <string>
#include <list>
#include <map>



class PolyTransComHelper
{
public:
    typedef std::list<std::string> ExtensionList;
    typedef std::map<std::string,std::string> PluginMap;

    //CONSTRUCTOR
    PolyTransComHelper();


    //DESTRUCTOR
    ~PolyTransComHelper(void);


    //ACCESSORS
    void SetExportPath( std::string& exportPath );
    void SetAppWindowName( const std::string& anAppWindowName );
    void SetShowExportOptionsWindow( bool showWindow );
    void SetShowImportOptionsWindow( bool showWindow );
    void setPluginPreferences( const PluginMap& polyTransPluginPreferences );
    void setIntermediateFileNameExt( const std::string& ext );
    void setIntermediateFileNameBase( const std::string& base );
    std::string GetLowerCaseFullFileExtension( const std::string& aFileName );
    std::string ComputeIntermediateFileNameAndPath( const std::string& srcFile="" ) const;
    std::string ComputeGeneratedFileNameAndPath();
    ExtensionList getSupportedExtensions();

    //UTILITIES
    bool AttachToPolyTransCom( HINSTANCE anAppInstance );
    void DetachFromPolyTransCOM();
    bool ExportPolyTransModelToIntermediateFile();
    bool ExportPolyTransModelToOpenFlight();
    bool ExportPolyTransModelToWaveFront();
    bool ImportModelIntoPolyTrans( const std::string& aFileNameAndPathToImport );
    bool ResetPolyTrans();

private:
    //UTILITIES
    int  FindPolyTransIniCreateNewFolderValue( const std::string& aPolyTransIniFileAndPath );
    bool RegisterPolyTransComServer();
    void SearchListOfExporters();
    void SearchListOfImporters();
    std::string GetGUIDForImporterName( const std::string& anImporterName );


    //DATA MEMBERS
    bool m_ShowImportOptionsWindow;
    bool m_ShowExportOptionsWindow;
    std::string m_AppWindowName;
    std::string m_FileNameAndPathToImport;
    std::string m_exportPath;
    std::string m_ConfigValue_BaseIntermediateFileName;
    std::string m_ConfigValue_IntermediateFileType;
    const std::string m_ConfigProperty_BaseIntermediateFileName;
    const std::string m_ConfigProperty_IntermediateFileType;
    const std::string m_ConfigProperty_PolyTransINIFileName;

    PluginMap m_PolyTransImporters;
};
