#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include "osgOQ/OcclusionQueryContext.h"
#include "osgOQ/OcclusionQueryRoot.h"

#include "COMHelper/PolyTransComHelper.h"
#include "osgOQ/ConfigFileReader.h"
#include <string>
#include <sstream>


class ReaderWriterPolyTrans : public osgDB::ReaderWriter
{
public:
    virtual const char* className() const { return "PolyTrans Reader"; }

    virtual bool acceptsExtension( const std::string& extension ) const
    {
		// If we are already loading a file, DO NOT claim to support
		//   ANY other file types.
		if (_loading)
			return false;

		PolyTransComHelper polyTransComHelper;
		if (!polyTransComHelper.AttachToPolyTransCom( NULL ))
		{
			osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed to attach to PolyTrans via Okino COM interface." << std::endl;
			return false;
		}

		return( polyTransComHelper.IsExtensionSupportedByImporters( extension ) );
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
			return ReadResult::FILE_NOT_HANDLED;

		// Look for a config file. The config file changes this plugin's
		//   default behavior. To use a config file, set the OSG_POLYTRANS_CONFIG_FILE
		//   env var to the config file name.
		ConfigFileReader configFileReader;
		{
			std::string envStr( "OSG_POLYTRANS_CONFIG_FILE" );
			char* charPtr = getenv( envStr.c_str() );
			if (charPtr)
			{
				std::string configFile = std::string( charPtr );
				std::string fullName = osgDB::findDataFile( configFile, options );
				bool setSuccess( false );
				if (!fullName.empty())
					setSuccess = configFileReader.SetConfigFileName( fullName );
				if (!setSuccess)
					osg::notify(osg::INFO) << "osgdb_PolyTrans: Can't load " << envStr << ": \"" << fullName << "\"." << std::endl;
			}
		}

        // TBD Eventually, parse options out of the Options and override
        //   ConfigFile values. No options supported right now.
#if 0
        // Parse options
		bool bufferSizeOption( false );
		int bufferSize;
        if (options) {
            const std::string optStr = options->getOptionString();
			// TBD really should just be able to send the option string to the
			//   ConfigFileReader and let it parse the string and override
			//   values. Will need to revamp CFR to add that functionality.
			// Hm. Maybe need to pass option string to OQC...
            if (optStr.find( "BufferSize" ) != std::string::npos)
			{
				bufferSizeOption = true;
				bufferSize = findOption( "BufferSize", optStr );
            }
        }
#endif

		// Create the COMHelper object and configure it
		PolyTransComHelper polyTransComHelper;
		if (!polyTransComHelper.AttachToPolyTransCom( NULL ))
		{
			osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed to attach to PolyTrans via Okino COM interface." << std::endl;
			return ReadResult::FILE_NOT_HANDLED;
		}

		if (configFileReader.valid())
		{
			polyTransComHelper.LoadOptionsFromConfigFile( configFileReader );
		}


		// Get the extension and full path / file name 
        std::string ext = findExtension( file, &polyTransComHelper );
		bool isSupported = polyTransComHelper.IsExtensionSupportedByImporters( ext );
		if ( !isSupported )
		{
            return ReadResult::FILE_NOT_HANDLED;
		}

        // Do not handle an input file if its extension matches
        //   the intermediate file type.
		if ( configFileReader.HasProperty( _intermediateFileTypeToken ) )
		{
			// Get config file value to specify cached or non-cached load.
			std::string valueStr = configFileReader.GetValue( _intermediateFileTypeToken );
            if (osgDB::equalCaseInsensitive( valueStr, ext ) )
                return ReadResult::FILE_NOT_HANDLED;
        }
        // handle default case
        else if (osgDB::equalCaseInsensitive( "flt", ext ) )
        {
            return ReadResult::FILE_NOT_HANDLED;
        }
        // handle WaveFront files case
        else if (osgDB::equalCaseInsensitive( "obj", ext ) )
        {
            return ReadResult::FILE_NOT_HANDLED;
        }
        // handle STL case
        else if (osgDB::equalCaseInsensitive( "stl", ext ) )
        {
            return ReadResult::FILE_NOT_HANDLED;
        }
        
		std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty())
		{
            return ReadResult::FILE_NOT_FOUND;
		}
		polyTransComHelper.SetExportPath( osgDB::getFilePath( fileName ) );

		// Indicate that a load is in progress. When set to true,
		//   this plugin will not allow a new load and will not
		//   tell OSG that it supports any extensions.
		_loading = true;

		// Check for ability to load from cache
		bool cachedLoad = getLoadFromCache( configFileReader, fileName,
			polyTransComHelper.ComputeIntermediateFileNameAndPath( fileName ) );

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
				return ReadResult::FILE_NOT_HANDLED;
			}


			//This function will export the model that is currently loaded in PolyTrans into an 
			// OpenFlight file. The default behavior is to use the same filename as the file being
			// imported, but with the extension .flt.  You can use a COMHelper config file property
			// to set a .flt filename.
			if ( !polyTransComHelper.ExportPolyTransModelToIntermediateFile())
			{
				osg::notify(osg::FATAL) << "osgdb_PolyTrans:: Failed PolyTrans conversion. Can't export to intermediate file." << std::endl;
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


		// Check to see if we should delete the intermediate file.
		bool delFile( false );
		if ( configFileReader.HasProperty( _deleteIntermediateFileToken ) )
		{
			std::string valueStr = configFileReader.GetValue( _deleteIntermediateFileToken );
			bool value;
			if (ConfigFileReader::convertToBool( value, valueStr ))
				delFile = value;
		}

		if (delFile)
		{
			// Delete the intermediate file.
			// TODO: we might want to delete the new folder that may have been created by PolyTrans to
			// put the .flt file in.
			BOOL deleteSuccess = DeleteFile( intermediateFile.c_str() );
		}


		if (readNodeResult.getNode() != NULL)
		{
			// Add occlusion query support to the model.
			//osg::ref_ptr<osgOQ::OcclusionQueryContext> oqc = new osgOQ::OcclusionQueryContext;
            /* TBD
			if (bufferSizeOption)
				oqc->setBufferSize( bufferSize );
            */

			//osg::ref_ptr<osgOQ::OcclusionQueryRoot> oqr = new osgOQ::OcclusionQueryRoot( oqc.get() );
			//oqr->addChild( readNodeResult.getNode() );
			//readNodeResult = oqr.get();
		}

		return readNodeResult;
    }

protected:
	// PTC Pro/Engineer appends a revision number to file names, such as "model.prt.1".
	// Once this plugin is loaded, it gets a crack at loading every file that the app
	// tries to load. This function strips the revision extension to see if it's a
	// file we support. If the extension isn't a revision (e.g., "image.gif") then this
	// function still returns some string which is probably not a supported file format.
	std::string findExtension( const std::string& fName, PolyTransComHelper* aPolyTransComHelper ) const
	{
		std::string ext = osgDB::getFileExtension( fName );
		if ( !aPolyTransComHelper->IsExtensionSupportedByImporters( ext ) )
		{
			// Probably we just found a revision number: model.asm.2, for example.
			// Strip this extension off and see if we can find another one.
			std::string shortName = osgDB::getNameLessExtension( fName );
			ext = osgDB::getFileExtension( shortName );
		}
		return ext;
	}

	static int findOption( const std::string& target, const std::string& optStr )
	{
		int n = optStr.find( target );
		if (n == optStr.npos)
			return -1;

		std::istringstream sOpt( optStr.substr(n) );
		std::string targetX;
		int value;
		sOpt >> targetX;
		sOpt >> value;
		return value;
	}

	static bool getLoadFromCache( ConfigFileReader& cfr, const std::string& srcFile, const std::string& destFile )
	{
		// Check cache file existance
		if (!osgDB::fileExists( destFile ))
		{
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Can't find cache file \"" << destFile <<
				"\". Will convert from source file." << std::endl;
			return false;
		}

		// Determine whether we should try to load from cache.
		//   The default behavior is 'true'.
		bool cachedLoad( true );
		if ( cfr.HasProperty( _cachedLoadToken ) )
		{
			// Get config file value to specify cached or non-cached load.
			std::string valueStr = cfr.GetValue( _cachedLoadToken );
			bool value;
			if (ConfigFileReader::convertToBool( value, valueStr ))
				cachedLoad = value;
		}
		if (!cachedLoad)
		{
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Cache disabled in cnfig file. " <<
				"Will convert from source file." << std::endl;
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
		cachedLoad = (result == -1 ); // -1 means first file time is earier than second.

		if (!cachedLoad)
			osg::notify( osg::INFO ) << "osgdb_PolyTrans: Source file newer than cache file. " <<
				"Will convert from source file." << std::endl;
#endif

		return cachedLoad;
	}

	// Static config file tokens
	static std::string _deleteIntermediateFileToken;
	static std::string _cachedLoadToken;
    static std::string _intermediateFileTypeToken;

	// Static var to prevent this plugin from attempting to
	//   use PolyTrans to load the intermediate file.
	static bool _loading;
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterPolyTrans> g_readerWriter_PolyTrans_Proxy;


// Define config file tokens
std::string ReaderWriterPolyTrans::_deleteIntermediateFileToken( "DeleteIntermediateFile" );
std::string ReaderWriterPolyTrans::_cachedLoadToken( "CachedLoad" );
std::string ReaderWriterPolyTrans::_intermediateFileTypeToken( "COMHelper_IntermediateFileType" );

// Initially, not loading. Will toggle to true during file load.
bool ReaderWriterPolyTrans::_loading( false );
