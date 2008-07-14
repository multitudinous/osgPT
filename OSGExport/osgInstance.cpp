
#include "main.h"
#include "osgInstance.h"

#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osg/Node>
#include <osg/ProxyNode>
#include <osg/ref_ptr>
#include <string>
#include <map>


typedef std::map< std::string, osg::ref_ptr< osg::Node > > InstanceMap;
static InstanceMap _instanceMap;


std::string createFileName( const std::string& handle, const std::string& extension )
{
    // The original PTC file name is embedded into the handdle name.
    // However, each instance of the object is appended with " #<n>",
    // where <n> is "2", "3", etc., for as many instances as are present.
    // This function strips the " #<n>" suffix and appends the specified
    // extension in its place.
    // 
    // As an example, if the input is:
    //   handle "origfile-5.prt #3"
    //   extension "ive"
    // The return value will be:
    //   "origfile-5.prt.ive"
    std::string::size_type pos = handle.find_last_of( "#" );
    if ( pos != handle.npos)
        return( handle.substr( 0, pos-1 ) + "." + extension );
    else
        return( handle + "." + extension );
}


osg::Node*
findNodeForInstance( const std::string& name )
{
    InstanceMap::iterator it = _instanceMap.find( name );
    if (it != _instanceMap.end())
        return( (it->second).get() );
    else
        return NULL;
}

osg::Node*
createReferenceToInstance( const std::string& objectName, const std::string& sourceFileName, const std::string& extension )
{
    if (export_options->osgInstanceFile)
    {
        osg::ProxyNode* pn = new osg::ProxyNode;
        pn->setFileName( 0, createFileName( sourceFileName, extension ) );
        return( pn );
    }
    else if (export_options->osgInstanceShared)
    {
#ifdef _DEBUG
        // DEBUG only -- verify that we really should be here.
        osg::Node* root = findNodeForInstance( objectName );
        if (root == NULL)
        {
            Ni_Report_Error_printf( Nc_ERR_WARN, "createReferenceToInstance: can't find %s\n", objectName.c_str() );
            return NULL;
        }
#endif
        return( _instanceMap[ objectName ].get() );
    }
    else
        return NULL;
}

bool
addInstance( const std::string& name, osg::Node* root )
{
    _instanceMap[ name ] = root;
    return true;
}

unsigned int
getNumberOfInstances()
{
    return( _instanceMap.size() );
}

void
writeInstancesAsFiles( const std::string& extension, const osgDB::ReaderWriter::Options* opt )
{
    // Possibly run the Optimizer
    unsigned int flags( 0 );
    if (export_options->osgRunOptimizer)
    {
        if (export_options->osgCreateTriangleStrips)
            flags |= osgUtil::Optimizer::TRISTRIP_GEOMETRY;
        if (export_options->osgMergeGeometry)
            flags |= osgUtil::Optimizer::MERGE_GEOMETRY;
    }

    int count( 0 );
    InstanceMap::iterator it = _instanceMap.begin();
    for( ; it != _instanceMap.end(); it++)
    {
        osg::Node* node = it->second.get();

        osgUtil::Optimizer optimizer;
        optimizer.optimize( node, flags );

        const std::string sourceFileName = node->getName();
        const std::string fileName = createFileName( sourceFileName, extension );

        if (osgDB::fileExists( fileName ))
            Ni_Report_Error_printf( Nc_ERR_WARNING, "writeInstancesAsFiles: File already exists: \"%s\".", fileName.c_str() );

        Export_IO_UpdateStatusDisplay( "instance", (char *)(fileName.c_str()), "Exporting instance as file." );
        bool success = osgDB::writeNodeFile( *node, fileName, opt );
        if (!success)
            Ni_Report_Error_printf( Nc_ERR_INFO, "writeInstancesAsFiles: Error writing instance file \"%s\".", fileName.c_str() );

        count++;

        // Update progress bar and check for user aboirt.
        if (Export_IO_Check_For_User_Interrupt_With_Stats( count, _instanceMap.size() ))
            return;
    }
}

void
clearInstances()
{
    _instanceMap.clear();
}
