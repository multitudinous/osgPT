#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "COMHelper/PolyTransComHelper.h"
#include "OptionLoader.h"
#include <string>
#include <sstream>


class ReaderWriterPolyTrans : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const { return "PolyTrans Reader"; }

    ReaderWriterPolyTrans()
      : _intermediateFileNameExt( "flt" ),
        _deleteIntermediateFile( false ),
        _cachedLoad( true ),
        _showImportOptions( true ),
        _showExportOptions( true )
    {
        _rejectExtensions.push_back( "osga" );
        _rejectExtensions.push_back( "osg" );
        _rejectExtensions.push_back( "ive" );
        _rejectExtensions.push_back( "ttf" );

        // Find and load the PolyTrans config file.
        //   An example config file with documentation is
        //   located in the sibling data directory.
		std::string envStr( "OSG_POLYTRANS_CONFIG_FILE" );
		if (char* charPtr = getenv( envStr.c_str() ))
		{
			std::string fullName = osgDB::findDataFile( std::string( charPtr ) );
			if (!fullName.empty())
            {
				osg::notify( osg::INFO ) << "osgdb_PolyTrans: Loading " << envStr << ": \"" << fullName << "\"." << std::endl;

                OptionLoader baseOptions;
                std::ifstream in( fullName.c_str() );
                if (!baseOptions.load( in ))
                {
                    osg::notify( osg::WARN ) << "osgdb_PolyTrans: Some options failed to load from config file: \"" << fullName << "\"." << std::endl;
                }
                in.close();

                baseOptions.getOption( _optionAppWindowName, _appWindowName );
                baseOptions.getOption( _optionIntermediateFileNameBase, _intermediateFileNameBase );
                baseOptions.getOption( _optionIntermediateFileNameExt, _intermediateFileNameExt );
                baseOptions.getOption( _optionDeleteIntermediateFile, _deleteIntermediateFile );
                baseOptions.getOption( _optionCachedLoad, _cachedLoad );
                baseOptions.getOption( _optionShowImportOptions, _showImportOptions );
                baseOptions.getOption( _optionShowExportOptions, _showExportOptions );

                std::string tmpString;
                baseOptions.getOption( _optionRejectExtensions, tmpString );
                parseExtensionList( _rejectExtensions, tmpString );
                baseOptions.getOption( _optionPolyTransPluginPreference, tmpString );
                parsePluginMap( _polyTransPluginPreference, tmpString );
            }
            else
            {
				osg::notify(osg::WARN) << "osgdb_PolyTrans: Can't load " << envStr << ": \"" << fullName << "\"." << std::endl;
            }
		}
    }


    bool rejectsExtension( const std::string& extension, const PolyTransComHelper::ExtensionList& rejectList ) const
    {
        // This function quickly rejects extensions that are known
        //   to not be supported by PolyTrans. This avoids having to
        //   launch PolyTrans via COM to find out the extension isn't
        //   supported.

        PolyTransComHelper::ExtensionList::const_iterator it = rejectList.begin();
        while (it != rejectList.end())
        {
            if (osgDB::equalCaseInsensitive( extension, *it ) )
            {
                return true;
            }
            it++;
        }
        return false;
    }

    virtual bool acceptsExtension( const std::string& extension ) const
    {
		// If we are already loading a file, DO NOT claim to support
		//   ANY other file types.
		if (_loading)
        {
			return false;
        }

        if (rejectsExtension( extension, _rejectExtensions ) )
        {
            return false;
        }

        if (!internalAccept( extension ))
        {
            return false;
        }

        return true;
    }

    virtual ReadResult readObject( const std::string& file, const Options* options ) const
    {
        return readNode( file, options );
    }
    
	virtual ReadResult readNode( const std::string& file, const Options* options ) const
    {
		// If we are already loading a file, do not initiate
		//   a new load. This ensures we do not launch PolyTrans
		//   in support of a cached load.
		if (_loading)
        {
			return ReadResult::FILE_NOT_HANDLED;
        }


        // Parse options string. This overrides any default settings
        //   or settings in the config file.
        // Option strings are semi-colon separated strings that otherwise
        //   confom to the syntax rules in the example PolyTrans config file.
        //   For example, a valid option string might be:
        //     IntermediateFileNameExt obj;DeleteIntermediateFile true;CachedLoad false
        OptionLoader overrides;
        std::string allOptions;
        if (options)
            allOptions = options->getOptionString();
        std::string::size_type startIdx( 0 );
        std::string::size_type endIdx = allOptions.find_first_of( ";" );
        while( (startIdx < endIdx) && !allOptions.empty() )
        {
            const std::string singleOpt = allOptions.substr( startIdx, endIdx-startIdx );

            std::istringstream istr( singleOpt );
            if (!overrides.load( istr ))
            {
                osg::notify( osg::WARN ) << "osgdb_PolyTrans: Some options failed to load from osgDB::Options." << std::endl;
            }

            if (endIdx == allOptions.npos)
                break;

            startIdx = endIdx+1;
            endIdx = allOptions.find_first_of( ";", startIdx );
        }

        std::string appWindowName( _appWindowName );
        std::string intermediateFileNameBase( _intermediateFileNameBase );
        std::string intermediateFileNameExt( _intermediateFileNameExt );
        bool deleteIntermediateFile( _deleteIntermediateFile );
        bool cachedLoad( _cachedLoad );
        bool showImportOptions( _showImportOptions );
        bool showExportOptions( _showExportOptions );
        PolyTransComHelper::ExtensionList rejectExtensions( _rejectExtensions );
        PolyTransComHelper::PluginMap polyTransPluginPreference( _polyTransPluginPreference );

        overrides.getOption( _optionAppWindowName, appWindowName );
        overrides.getOption( _optionIntermediateFileNameBase, intermediateFileNameBase );
        overrides.getOption( _optionIntermediateFileNameExt, intermediateFileNameExt );
        overrides.getOption( _optionDeleteIntermediateFile, deleteIntermediateFile );
        overrides.getOption( _optionCachedLoad, cachedLoad );
        overrides.getOption( _optionShowImportOptions, showImportOptions );
        overrides.getOption( _optionShowExportOptions, showExportOptions );

        std::string tmpString;
        overrides.getOption( _optionRejectExtensions, tmpString );
        parseExtensionList( rejectExtensions, tmpString );
        overrides.getOption( _optionPolyTransPluginPreference, tmpString );
        parsePluginMap( polyTransPluginPreference, tmpString );

        // Also reject the intermediate file name extension, if specified.
        if (!intermediateFileNameExt.empty())
        {
            rejectExtensions.push_back( intermediateFileNameExt );
        }


		// Get the extension and full path / file name 
        std::string ext = findExtension( file );
        //   See if PolyTrans accepts it
		if ( !internalAccept( ext ) )
		{
            osg::notify( osg::INFO ) << "osgdb_PolyTrans: Not supported by PolyTrans: \"" << ext;
            osg::notify( osg::INFO ) << " \". Consider using \"RejectExtensions\" option." << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
		}
        //   Reject any explicitly-rejected extensions.
        if (rejectsExtension( ext, rejectExtensions ))
        {
            return ReadResult::FILE_NOT_HANDLED;
        }

		std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty())
		{
            return ReadResult::FILE_NOT_FOUND;
		}

        
        // Create the COMHelper object and configure it
		PolyTransComHelper polyTransComHelper;
		if (!polyTransComHelper.AttachToPolyTransCom( NULL ))
		{
			osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed to attach to PolyTrans via Okino COM interface." << std::endl;
			return ReadResult::FILE_NOT_HANDLED;
		}

        if (!appWindowName.empty())
            polyTransComHelper.SetAppWindowName( appWindowName );
        polyTransComHelper.SetShowImportOptionsWindow( showImportOptions );
        polyTransComHelper.SetShowExportOptionsWindow( showExportOptions );
        polyTransComHelper.setPluginPreferences( polyTransPluginPreference );
        polyTransComHelper.setIntermediateFileNameBase( intermediateFileNameBase );
        polyTransComHelper.setIntermediateFileNameExt( intermediateFileNameExt );

		polyTransComHelper.SetExportPath( osgDB::getFilePath( fileName ) );


		// Indicate that a load is in progress. When set to true,
		//   this plugin will not allow a new load and will not
		//   tell OSG that it supports any extensions.
		_loading = true;

        if (cachedLoad)
        {
    		// Check for ability to load from cache
		    cachedLoad = getLoadFromCache( fileName, polyTransComHelper.ComputeIntermediateFileNameAndPath( fileName ) );
        }
        else
        {
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Cache disabled in config file. " <<
				"Will convert from source file." << std::endl;
        }


		std::string intermediateFile;
		if (cachedLoad)
		{
			intermediateFile = polyTransComHelper.ComputeIntermediateFileNameAndPath( fileName );
		}
		else
		{
			// Import the input file.
			bool loaded = polyTransComHelper.ImportModelIntoPolyTrans( fileName );
			if (!loaded)
			{
				osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed to import file: \"" << fileName << "\"." << std::endl;
        		_loading = false;
				return ReadResult::FILE_NOT_HANDLED;
			}


			//This function will export the model that is currently loaded in PolyTrans into an 
			// OpenFlight file. The default behavior is to use the same filename as the file being
			// imported, but with the extension .flt.  You can use a COMHelper config file property
			// to set a .flt filename.
			if ( !polyTransComHelper.ExportPolyTransModelToIntermediateFile())
			{
				osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed PolyTrans conversion. Can't export to intermediate file." << std::endl;
        		_loading = false;
				return ReadResult::FILE_NOT_HANDLED;
			}

			bool resetSuccess = polyTransComHelper.ResetPolyTrans();

			// Get the name of the intermediate file.
			intermediateFile = polyTransComHelper.ComputeGeneratedFileNameAndPath();
		}
		
		// Shut down PolyTrans (no longer needed). This will conserve memory for the
		// next step, loading the intermediate file.
		polyTransComHelper.DetachFromPolyTransCOM();

		// Get the plugin specifically for the intermediate file type
		osg::ref_ptr<osgDB::ReaderWriter> reader = 
			osgDB::Registry::instance()->getReaderWriterForExtension(
			osgDB::getFileExtension( intermediateFile ));
		if( !reader.valid() )
		{
			osg::notify(osg::FATAL) << "osgdb_PolyTrans: Unable to find OSG plugin to load intermediate file: \"" << intermediateFile << "\"." << std::endl;
			return ReadResult::FILE_NOT_HANDLED;
		}

		// Use the plugin to load our intermediate file.
		osgDB::ReaderWriter::ReadResult readNodeResult = reader->readNode( intermediateFile );

		// Load is complete.
		_loading = false;


		if (deleteIntermediateFile)
		{
			// Delete the intermediate file.
			// TODO: we might want to delete the new folder that may have been created by PolyTrans to
			// put the .flt file in.
			BOOL deleteSuccess = DeleteFile( intermediateFile.c_str() );
		}

		return readNodeResult;
    }

protected:
    // Config file and osgDB::ReaderWriterOptions options.
    std::string _appWindowName;
    std::string _intermediateFileNameBase;
    std::string _intermediateFileNameExt;
    bool _deleteIntermediateFile;
    bool _cachedLoad;
    bool _showImportOptions;
    bool _showExportOptions;
    PolyTransComHelper::ExtensionList _rejectExtensions;
    PolyTransComHelper::PluginMap _polyTransPluginPreference;

    // Config file and osgDB::ReaderWriterOptions options tokens.
    static std::string _optionAppWindowName;
    static std::string _optionIntermediateFileNameBase;
    static std::string _optionIntermediateFileNameExt;
    static std::string _optionDeleteIntermediateFile;
    static std::string _optionCachedLoad;
    static std::string _optionShowImportOptions;
    static std::string _optionShowExportOptions;
    static std::string _optionRejectExtensions;
    static std::string _optionPolyTransPluginPreference;

    static void parseExtensionList( PolyTransComHelper::ExtensionList& el, const std::string& str )
    {
        if (str.empty())
            return;

        std::string::size_type startIdx( 0 );
        std::string::size_type endIdx = str.find_first_of( " " );
        while (startIdx < endIdx)
        {
            const std::string ext = str.substr( startIdx, endIdx-startIdx );
            el.push_back( ext );
            if (endIdx == str.npos)
                break;

            startIdx = endIdx+1;
            endIdx = str.find_first_of( " ", startIdx );
        }
    }

    static void parsePluginMap( PolyTransComHelper::PluginMap& pm, const std::string& str )
    {
        if (str.empty())
            return;

        std::string::size_type startIdx( 0 );
        std::string::size_type endIdx = str.find_first_of( "," );
        while (startIdx < endIdx)
        {
            std::string::size_type spIdx = str.find_first_of( " ", startIdx );
            const std::string ext = str.substr( startIdx, spIdx-startIdx );
            const std::string pluginName = str.substr( spIdx+1, endIdx-(spIdx+1) );
            pm[ ext ] = pluginName;
            if (endIdx == str.npos)
                break;

            startIdx = endIdx+1;
            endIdx = str.find_first_of( ",", startIdx );
        }
    }


	// PTC Pro/Engineer appends a revision number to file names, such as "model.prt.1".
	// Once this plugin is loaded, it gets a crack at loading every file that the app
	// tries to load. This function strips the revision extension to see if it's a
	// file we support. If the extension isn't a revision (e.g., "image.gif") then this
	// function still returns some string which is probably not a supported file format.
	std::string findExtension( const std::string& fName ) const
	{
		std::string ext = osgDB::getFileExtension( fName );
		if ( !internalAccept( ext ) )
		{
			// Probably we just found a revision number: model.asm.2, for example.
			// Strip this extension off and see if we can find another one.
			std::string shortName = osgDB::getNameLessExtension( fName );
			ext = osgDB::getFileExtension( shortName );
		}
		return ext;
	}

	static bool getLoadFromCache( const std::string& srcFile, const std::string& destFile )
	{
		// Check cache file existence
		if (!osgDB::fileExists( destFile ))
		{
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Can't find cache file \"" << destFile <<
				"\". Will convert from source file." << std::endl;
			return false;
		}

		// Check file dates
#ifdef WIN32
		// Not implemented for non-Windows platforms.

		HANDLE srcHandle, destHandle;
		srcHandle = CreateFile( srcFile.c_str(),     // open One.txt 
			GENERIC_READ,                 // open for reading 
			0,                            // do not share 
			NULL,                         // no security 
			OPEN_EXISTING,                // existing file only 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);
		destHandle = CreateFile( destFile.c_str(),     // open One.txt 
			GENERIC_READ,                 // open for reading 
			0,                            // do not share 
			NULL,                         // no security 
			OPEN_EXISTING,                // existing file only 
			FILE_ATTRIBUTE_NORMAL,        // normal file 
			NULL);

		FILETIME srcCreate, srcAccess, srcWrite, destCreate, destAccess, destWrite;
		BOOL srcOK = GetFileTime( srcHandle, &srcCreate, &srcAccess, &srcWrite );
		BOOL destOK = GetFileTime( destHandle, &destCreate, &destAccess, &destWrite );
		if (!srcOK || !destOK)
		{
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Problem retrieving the file date. " <<
				"Will convert from source file." << std::endl;
			// Unable to get time info from files.
			return false;
		}

		CloseHandle( srcHandle );
		CloseHandle( destHandle );

		LONG result = CompareFileTime( &srcWrite, &destWrite );
		bool cachedLoad = ( result == -1 ); // -1 means first file time is earier than second.

		if (!cachedLoad)
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Source file newer than cache file. " <<
				"Will convert from source file." << std::endl;
#endif

		return cachedLoad;
	}

    // If our cache of supported extensions doesn't exist, launch PolyTrans
    //   to obtain the list of supported extensions.
    // Check the given extension against the supported extensions list and
    //   return true if supported by PolyTrans, false otherwise.
    bool internalAccept( const std::string& ext ) const
    {
        if (_supportedExtensions.empty())
        {
            PolyTransComHelper polyTransComHelper;
		    if (!polyTransComHelper.AttachToPolyTransCom( NULL ))
		    {
			    osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed to attach to PolyTrans via Okino COM interface." << std::endl;
			    return false;
		    }
            _supportedExtensions = polyTransComHelper.getSupportedExtensions();
        }

        PolyTransComHelper::ExtensionList::const_iterator it = _supportedExtensions.begin();
        while (it != _supportedExtensions.end())
        {
            if (osgDB::equalCaseInsensitive( ext, *it ) )
            {
                return true;
            }
            it++;
        }
        return false;
    }

	// Static var to prevent this plugin from attempting to
	//   use PolyTrans to load the intermediate file.
	static bool _loading;

    // List of supported extensions from PolyTrans.
    mutable PolyTransComHelper::ExtensionList _supportedExtensions;
};

// now register with Registry to instantiate the above
// reader/writer.
// TBD this is SG v1.2 method, need to switch over to v2.0 macro
osgDB::RegisterReaderWriterProxy<ReaderWriterPolyTrans> g_readerWriter_PolyTrans_Proxy;


// Initially, not loading. Will toggle to true during file load.
bool ReaderWriterPolyTrans::_loading( false );


std::string ReaderWriterPolyTrans::_optionAppWindowName( "AppWindowName" );
std::string ReaderWriterPolyTrans::_optionIntermediateFileNameBase( "IntermediateFileNameBase" );
std::string ReaderWriterPolyTrans::_optionIntermediateFileNameExt( "IntermediateFileNameExt" );
std::string ReaderWriterPolyTrans::_optionDeleteIntermediateFile( "DeleteIntermediateFile" );
std::string ReaderWriterPolyTrans::_optionCachedLoad( "CachedLoad" );
std::string ReaderWriterPolyTrans::_optionShowImportOptions( "ShowImportOptions" );
std::string ReaderWriterPolyTrans::_optionShowExportOptions( "ShowExportOptions" );
std::string ReaderWriterPolyTrans::_optionRejectExtensions( "RejectExtensions" );
std::string ReaderWriterPolyTrans::_optionPolyTransPluginPreference( "PolyTransPluginPreference" );
